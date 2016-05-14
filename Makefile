# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)
$(call check-var-defined,LIBUNWIND_BASE)

CFLAGS=-fPIC -shared -Wall -std=gnu99
OPT_FLAGS=-g -O3 -Wall

SRC=src/stack.c src/driver.c src/unwinding.c
LIBNAME=sampling

LDFLAGS=-lc
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread
LIBUNWIND_FLAGS=-I$(LIBUNWIND_BASE)/include -L$(LIBUNWIND_BASE)/lib -lunwind-x86_64 -lunwind
PAPI_FLAGS=-lpapi -pthread

STACKWALKER_BASE=/home/us93buza/opt/stackwalker/DyninstAPI-9.0.3
STACKWALKER_FLAGS=-I$(BOOST)/include \
	-I$(STACKWALKER_BASE)/proccontrol/h -I$(STACKWALKER_BASE)/common/h -I$(STACKWALKER_BASE)/build/common/h \
	-I$(STACKWALKER_BASE)/stackwalk/h -L$(STACKWALKER_BASE)/build/stackwalk -lstackwalk

INSTRO_FLAGS=-DMAX_SPEED
TARGET_FLAGS=-g -O0 -std=gnu99

libsampling-debug: PP_FLAGS+=-DPRINT_FUNCTIONS
libsampling libsampling-debug: PP_FLAGS+=$(PAPI_FLAGS) -DSERIAL_OPT -DMETA_BENCHMARK

libsampling libsampling-debug libshadowstack-serial libshadowstack-parallel \
measure measure-sampling-target measure-sampling-target-noHandler: libhash timing
	$(CC) $(PAPI_INCLUDE_FLAGS) $(PP_FLAGS) -I./src $(INSTRO_FLAGS) $(OPT_FLAGS) $(CFLAGS) -o lib/lib$(LIBNAME).$(CC).$(HOSTNAME).so $(SRC) -L./lib -ltiming_tsc $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libhash: $(eval LDFLAGS+=-lhash.$(CC).$(HOSTNAME))
libhash:
	$(CXX) -fPIC -shared $(OPT_FLAGS) -std=c++0x -Wall src/cpp/hash.cpp -o lib/libhash.$(CC).$(HOSTNAME).so

sampling-bench: measure measure-sampling-target measure-sampling-target-noHandler
#
measure: PP_FLAGS+=-DMETA_BENCHMARK -DNO_PAPI_DRIVER -DNO_PAPI_HANDLER
measure: LDFLAGS+=-ltiming_tsc
measure: LIBNAME=measure
# sampling overhead
measure-sampling: timing libshadowstack-serial
	$(CC) -std=gnu99 $(OPT_FLAGS)  -I./src -I$(LIBMONITOR_BASE)/include overhead/overhead-driver.c \
			-L./lib -ltiming -lhash -lshadowstack.serial.$(CC).$(HOSTNAME) $(PAPI_FLAGS) $(LIBUNWIND_FLAGS) $(LDFLAGS) -o overhead.exe
	python3 py/gen.py overhead.exe
#	taskset -c 5 monitor-run -i lib/libshadowstack.serial.$(CC).$(HOSTNAME).so overhead.exe
measure-sampling-target: PP_FLAGS+=-DMETA_BENCHMARK 
measure-sampling-target: LDFLAGS+=-ltiming_tsc
measure-sampling-target: LIBNAME=benchSampling
#
measure-sampling-target-noHandler: PP_FLAGS=-DMETA_BENCHMARK -DNO_PAPI_HANDLER
measure-sampling-target-noHandler: LDFLAGS+=-ltiming_tsc
measure-sampling-target-noHandler: LIBNAME=benchSampling.noHandler

# libunwind overhead without cache
measure-unw-nocache: PP_FLAGS:=-DNO_UNW_CACHE
measure-unw-nocache: measure-unw
# libunwind overhead with cache
measure-unw: LDFLAGS:=-L./lib -ltiming_tsc $(LD_FLAGS)
measure-unw: SRC=overhead/overhead-cyg_profile.c
measure-unw: LIBNAME=overhead
measure-unw: target timing libsampling
	taskset -c 5 monitor-run -i ./lib/lib$(LIBNAME).$(CC).$(HOSTNAME).so ./target.exe &> out
# stackwalker api overhead
measure-stackwalker: LIBNAME=stackwalker
measure-stackwalker: target timing
	$(CXX) $(PP_FLAGS) -std=gnu++11 $(OPT_FLAGS) -fPIC -shared -o lib/lib$(LIBNAME).$(CC).$(HOSTNAME).so overhead/overhead-stackwalker.cpp -I./src \
			$(LIBMONITOR_FLAGS)  $(STACKWALKER_FLAGS) -L./lib -ltiming_tsc
#	taskset -c 5 monitor-run -i ./lib/lib$(LIBNAME).$(CC).$(HOSTNAME).so ./target.exe &> out

# driver without papi sampling
libshadowstack-serial: PP_FLAGS+=-DSERIAL_OPT -DNO_PAPI_DRIVER -DNO_CPP_LIB -DMETA_BENCHMARK
libshadowstack-serial: LIBNAME=shadowstack.serial
# driver wihtout papi sampling
libshadowstack-parallel: PP_FLAGS+=-DNO_PAPI_DRIVER -DNO_CPP_LIB
libshadowstack-parallel: LIBNAME=shadowstack.parallel
# driver with itimer sampling
itimer: PP_FLAGS+=-DITIMER_DRIVER -DNO_PAPI_DRIVER -DSERIAL_OPT  -DMETA_BENCHMARK #-DMONITOR_INIT
itimer:	libsampling

# overhead of shadow stack (single/multi threaded)
measure-cyg: PP_FLAGS+=-DMETA_BENCHMARK -DMONITOR_INIT
measure-cyg: LDFLAGS+=-ltiming_tsc
measure-cyg: TARGET_FLAGS+=-DMETA_BENCHMARK
measure-cyg: target timing libempty libcount libshadowstack-parallel libshadowstack-serial
	python3 py/gen.py target.exe
	
libcount: target
	$(CC) $(OPT_FLAGS) $(CFLAGS) overhead/count-calls.c -o lib/libcount.so $(LIBMONITOR_FLAGS)
	
timing:
	$(CC) -O3 $(CFLAGS) src/libtiming/timing.c -o lib/libtiming.so -lrt
	$(CC) -O2 $(CFLAGS) src/libtiming/timing_papi.c -o lib/libtiming_papi.so -lrt -lpapi
	$(CC) -O2 $(CFLAGS) src/libtiming/timing_tsc.c -o lib/libtiming_tsc.so

libempty:	timing
	$(CC) -O3 $(CFLAGS) -DNO_INIT src/emptypushpop/emptypushpop.c -o lib/libempty.so
	$(CC) -O3 $(CFLAGS) $(PP_FLAGS) -DMETA_BENCHMARK src/emptypushpop/emptypushpop.c -o lib/libempty-monitor.so -I./src -L./lib -ltiming_tsc $(LIBMONITOR_FLAGS)
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack
	touch nm_file regions_file
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)
	./test_stack.exe

sampling: libsampling-debug
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 overhead/target.c -o target.exe
	python3 py/gen.py target.exe
	monitor-run -i ./lib/libsampling.$(CC).$(HOSTNAME).so ./target.exe

#target: EXCLUDE=-finstrument-functions-exclude-function-list=main,foo
target:
	$(CC) $(TARGET_FLAGS) -fno-inline -finstrument-functions $(EXCLUDE) overhead/target.c -o target.exe
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline -finstrument-functions $(EXCLUDE) overhead/target.c -o target-big.exe
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline -finstrument-functions -finstrument-functions-exclude-function-list="rec10,rec9,rec8,rec7,rec6" $(EXCLUDE) overhead/target.c -o target-sel.exe
	$(CC) $(TARGET_FLAGS) -fno-inline -finstrument-functions $(EXCLUDE) overhead/target-simple.c -o target-simple.exe
	
	$(CC) $(TARGET_FLAGS) -fno-inline overhead/target.c -o target.noinstr.exe
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline overhead/target.c -o target-big.noinstr.exe
	$(CC) $(TARGET_FLAGS) -fno-inline overhead/target-simple.c -o target-simple.noinstr.exe
	
	# scorep benchmark
#	scorep $(CC) -std=gnu99 -O3 -DMETA_BENCHMARK -fno-inline overhead/target.c -o target.scorep.$(HOSTNAME).exe
#	scorep $(CC) -std=gnu99 -O3 -DMETA_BENCHMARK -fno-inline -finstrument-functions-exclude-function-list=rec overhead/target.c -o target.scorep.filter.exe

vanilla:
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline overhead/target.c -o target.$(CC).vanilla
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline overhead/target-simple.c -o target-simple.$(CC).vanilla
instr:
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline -finstrument-functions $(EXCLUDE) overhead/target.c -o target.$(CC).instr
	$(CC) $(TARGET_FLAGS) -DMETA_BENCHMARK -fno-inline -finstrument-functions $(EXCLUDE) overhead/target-simple.c -o target-simple.$(CC).instr
	
.PHONY : clean target
clean:
	rm -f lib/*.so
	rm -f *.exe
#	rm -f nm_file regions_file map_file stack_file
