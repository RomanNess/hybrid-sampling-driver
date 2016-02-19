# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)
$(call check-var-defined,LIBUNWIND_BASE)

CC=gcc
CFLAGS=-fPIC -shared -Wall -std=gnu99
OPT_FLAGS=-g -O3 -Wall

SRC=src/stack.c src/driver.c src/unwinding.c
LIBNAME=sampling

LDFLAGS=-lc
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread
LIBUNWIND_FLAGS=-I$(LIBUNWIND_BASE)/include -L$(LIBUNWIND_BASE)/lib -lunwind-x86_64 -lunwind
PAPI_FLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -pthread

STACKWALKER_BASE=/home/us93buza/opt/stackwalker/DyninstAPI-9.0.3
STACKWALKER_FLAGS=-I$(BOOST)/include \
	-I/home/us93buza/opt/stackwalker/DyninstAPI-9.0.3/proccontrol/h \
	-I/home/us93buza/opt/stackwalker/DyninstAPI-9.0.3/common/h \
	-I/home/us93buza/opt/stackwalker/DyninstAPI-9.0.3/build/common/h \
	-I$(STACKWALKER_BASE)/stackwalk/h -L$(STACKWALKER_BASE)/build/stackwalk -lstackwalk

INSTRO_FLAGS=-DMAX_SPEED
TARGET_FLAGS=-g -O0 -std=gnu99

libsampling-debug: PP_FLAGS+=-DPRINT_FUNCTIONS

libsampling libsampling-debug libshadowstack-serial libshadowstack-parallel \
measure measure-sampling-target measure-sampling-target-noHandler: libhash timing
	$(CC) $(PAPI_INCLUDE_FLAGS) $(PP_FLAGS) -I./src $(INSTRO_FLAGS) $(OPT_FLAGS) $(CFLAGS) -o lib/lib$(LIBNAME).$(HOSTNAME).so $(SRC) -L./lib $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)
	#ln -s lib/lib$(LIBNAME).$(HOSTNAME).so lib/lib$(LIBNAME).so

libhash: $(eval LDFLAGS+=-lhash)
libhash:
	g++ -fPIC -shared $(OPT_FLAGS) -std=c++0x -Wall src/cpp/hash.cpp -o lib/libhash.so

sampling-bench: measure measure-sampling-target measure-sampling-target-noHandler

measure: PP_FLAGS+=-DMETA_BENCHMARK -DNO_PAPI_DRIVER -DNO_PAPI_HANDLER
measure: LDFLAGS+=-ltiming_tsc
measure: LIBNAME=measure
	
measure-sampling: timing libshadowstack-serial
	$(CC) -std=gnu99 $(OPT_FLAGS)  -I./src -I$(LIBMONITOR_BASE)/include overhead/overhead-driver.c \
			-L./lib -ltiming -lhash -lshadowstack.serial.$(HOSTNAME) $(PAPI_FLAGS) $(LIBUNWIND_FLAGS) $(LDFLAGS) -o overhead.exe
	python3 py/gen.py overhead.exe
#	taskset -c 13 bash ./preload.sh overhead.exe

measure-sampling-target: PP_FLAGS+=-DMETA_BENCHMARK 
measure-sampling-target: LDFLAGS+=-ltiming_tsc
measure-sampling-target: LIBNAME=benchSampling
	
measure-sampling-target-noHandler: PP_FLAGS=-DMETA_BENCHMARK -DNO_PAPI_HANDLER
measure-sampling-target-noHandler: LDFLAGS+=-ltiming_tsc
measure-sampling-target-noHandler: LIBNAME=benchSampling.noHandler

measure-unw-nocache: PP_FLAGS:=-DNO_UNW_CACHE
measure-unw-nocache: measure-unw

measure-unw: LDFLAGS:=-L./lib -ltiming_tsc $(LD_FLAGS)
measure-unw: SRC=overhead/overhead-cyg_profile.c
measure-unw: LIBNAME=overhead
measure-unw: target timing libsampling
	taskset -c 5 monitor-run -i ./lib/lib$(LIBNAME).$(HOSTNAME).so ./target.exe &> out
	
measure-unw-stackwalker: LDFLAGS:=-L./lib -ltiming_tsc $(LD_FLAGS)
measure-unw-stackwalker: SRC=overhead/overhead-stackwalker.cpp
measure-unw-stackwalker: LIBNAME=stackwalker
measure-unw-stackwalker: target timing
	g++ $(PP_FLAGS) -std=gnu++11 $(OPT_FLAGS) -fPIC -shared -Wall -o lib/lib$(LIBNAME).$(HOSTNAME).so $(SRC) -I./src \
			$(LIBMONITOR_FLAGS)  $(STACKWALKER_FLAGS) $(LDFLAGS) 
	taskset -c 5 monitor-run -i ./lib/lib$(LIBNAME).$(HOSTNAME).so ./target.exe &> out

libshadowstack-serial: PP_FLAGS+=-DSERIAL_OPT -DNO_PAPI_DRIVER
libshadowstack-serial: LIBNAME=shadowstack.serial

libshadowstack-parallel: PP_FLAGS+=-DNO_PAPI_DRIVER
libshadowstack-parallel: LIBNAME=shadowstack.parallel

measure-cyg: PP_FLAGS+=-DMETA_BENCHMARK
measure-cyg: LDFLAGS+=-ltiming_tsc
measure-cyg: TARGET_FLAGS+=-DMETA_BENCHMARK
measure-cyg: target timing libempty libshadowstack-parallel libshadowstack-serial
	python3 py/gen.py target.exe
	
count-calls: target
	$(CC) $(OPT_FLAGS) $(CFLAGS) overhead/count-calls.c -o lib/libcount.so $(LIBMONITOR_FLAGS)
#	LD_PRELOAD="./lib/libcount.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
#	LD_PRELOAD="./lib/libcount.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target-simple.exe
	
timing:
	$(CC) -O3 $(CFLAGS) src/libtiming/timing.c -o lib/libtiming.so -lrt
	$(CC) -O2 $(CFLAGS) src/libtiming_papi/timing.c -o lib/libtiming_papi.so -lrt -lpapi
	$(CC) -O2 $(CFLAGS) src/libtiming_tsc/timing.c -o lib/libtiming_tsc.so

libempty:	timing
	$(CC) -O3 $(CFLAGS) -DNO_INIT src/emptypushpop/emptypushpop.c -o lib/libempty.so
	$(CC) -O3 $(CFLAGS) $(PP_FLAGS) src/emptypushpop/emptypushpop.c -o lib/libempty-monitor.so -I./src -L./lib -ltiming_tsc $(LIBMONITOR_FLAGS)
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack
	touch nm_file regions_file
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)
	./test_stack.exe

sampling: libsampling-debug
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 overhead/target.c -o target.exe
	python3 py/gen.py target.exe
	LD_PRELOAD="./lib/libsampling.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	

#target: EXCLUDE=-finstrument-functions-exclude-function-list=main,foo
target:
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target.c -o target.exe
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target-simple.c -o target-simple.exe
	
	$(CC) $(TARGET_FLAGS) -fno-inline overhead/target.c -o target.noinstr.exe
	$(CC) $(TARGET_FLAGS) -fno-inline overhead/target-simple.c -o target-simple.noinstr.exe
	
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target-bigframe.c -o target-bigframe.exe

	# scorep benchmark
	scorep $(CC) -std=gnu99 -O3 -DMETA_BENCHMARK -fno-inline overhead/target.c -o target.scorep.$(HOSTNAME).exe
	scorep $(CC) -std=gnu99 -O3 -DMETA_BENCHMARK -fno-inline -finstrument-functions-exclude-function-list=rec overhead/target.c -o target.scorep.filter.exe
	
.PHONY : clean target
clean:
	rm -f lib/*.so
	rm -f *.exe
	rm -f nm_file regions_file map_file stack_file
