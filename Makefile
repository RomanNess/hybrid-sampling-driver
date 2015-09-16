# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)
$(call check-var-defined,LIBUNWIND_BASE)

CC=gcc
CFLAGS=-fPIC -shared -Wall -std=gnu99
OPT_FLAGS=-g -O3 -Wall

SRC=src/stack.c src/driver.c src/unwinding.c
LIBNAME=libsampling

LDFLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -pthread
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread
LIBUNWIND_FLAGS=-I$(LIBUNWIND_BASE)/include -L$(LIBUNWIND_BASE)/lib -lunwind-x86_64 -lunwind

INSTRO_FLAGS=-DMAX_SPEED
TARGET_FLAGS=-g -O0 -std=gnu99

libsampling-debug: PP_FLAGS+=-DPRINT_FUNCTIONS
libsampling-debug: libsampling

libsampling:
	$(CC) $(PAPI_INCLUDE_FLAGS) $(PP_FLAGS) -I./src $(INSTRO_FLAGS) $(OPT_FLAGS) $(CFLAGS) -o lib/$(LIBNAME).so $(SRC) -L./lib $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libhash: $(eval LDFLAGS+=-lhash)
libhash:
	g++ -fPIC -shared $(OPT_FLAGS) -std=c++0x -Wall src/cpp/hash.cpp -o lib/libhash.so

libshadowstack: PP_FLAGS+=-DNO_PAPI_DRIVER
libshadowstack: LIBNAME=libshadowstack
libshadowstack: libhash libsampling

measure: timing libhash libshadowstack
	$(CC) -std=gnu99 $(OPT_FLAGS) -I./src -I$(LIBMONITOR_BASE)/include overhead/overhead-driver.c -L./lib -ltiming -lhash -lshadowstack $(LIBUNWIND_FLAGS) $(LDFLAGS) -o overhead.exe
	python3 py/gen.py overhead.exe
#	LD_PRELOAD="./lib/libshadowstack.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./overhead.exe

measure-unw-nocache: PP_FLAGS:=-DNO_UNW_CACHE
measure-unw-nocache: measure-unw

measure-unw: PP_FLAGS+=-DNO_CPP_LIB -DNO_PAPI_DRIVER -DNO_CYG_PROF -DNO_MONITOR -DIGNORE_PAPI_CONTEXT
measure-unw: LDFLAGS:=-L./lib -ltiming_papi $(LD_FLAGS)
measure-unw: SRC+= overhead/overhead-cyg_profile.c
measure-unw: LIBNAME=liboverhead
measure-unw: target timing_papi libsampling
#	LD_PRELOAD="./lib/liboverhead.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe &> out

measure-cyg-serial: PP_FLAGS+=-DSERIAL_OPT
measure-cyg-serial: measure-cyg

measure-cyg: PP_FLAGS+=-DMETA_BENCHMARK
measure-cyg: LDFLAGS+=-ltiming_papi
measure-cyg: TARGET_FLAGS+=-DMETA_BENCHMARK
measure-cyg: target timing_papi libempty libhash libshadowstack
	python3 py/gen.py target.exe
#	taskset -c 5 preload.libshadowstack.sh ./target.exe
#	taskset -c 5 preload.libshadowstack.sh ./target.noinstr.exe
#	taskset -c 5 preload.libshadowstack.sh ./target-simple.exe
#	taskset -c 5 preload.libshadowstack.sh ./target-simple.noinstr.exe
#	taskset -c 5 preload.libempty.sh	./target.exe
#	taskset -c 5 preload.libempty.sh	./target-simple.exe

	
count-calls: target
	$(CC) $(OPT_FLAGS) $(CFLAGS) overhead/count-calls.c -o lib/libcount.so $(LIBMONITOR_FLAGS)
	LD_PRELOAD="./lib/libcount.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	LD_PRELOAD="./lib/libcount.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target-simple.exe
	
timing:
	$(CC) -O3 $(CFLAGS) libtiming/timing.c -o lib/libtiming.so -lrt
	
timing_papi:
	$(CC) -O2 $(CFLAGS) libtiming_papi/timing.c -o lib/libtiming_papi.so -lrt -lpapi

libempty:	timing_papi
	$(CC) -O3 $(CFLAGS) -DNO_MONITOR emptypushpop/emptypushpop.c -o lib/libempty.so
	$(CC) -O3 $(CFLAGS) $(PP_FLAGS) emptypushpop/emptypushpop.c -o lib/libempty-monitor.so -I. -L./lib -ltiming_papi $(LIBMONITOR_FLAGS)
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack
	touch nm_file regions_file
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)
	./test_stack.exe

sampling: libhash libsampling-debug
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 overhead/target.c -o target.exe
	python3 py/gen.py target.exe
	LD_PRELOAD="./lib/libsampling.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	

#target: EXCLUDE=-finstrument-functions-exclude-function-list=main,foo
target:
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target.c -o target.exe
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target-simple.c -o target-simple.exe
	
	$(CC) $(TARGET_FLAGS) overhead/target.c -o target.noinstr.exe
	$(CC) $(TARGET_FLAGS) overhead/target-simple.c -o target-simple.noinstr.exe
	
	$(CC) $(TARGET_FLAGS) -finstrument-functions $(EXCLUDE) overhead/target-bigframe.c -o target-bigframe.exe
	
.PHONY : clean
clean:
	rm -f lib/*.so
	rm -f *.exe
	rm -f nm_file regions_file map_file stack_file
