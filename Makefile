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

INSTRO_FLAGS=-DWITH_MAX_SIZE

libsampling-debug: PP_FLAGS+=-DPRINT_FUNCTIONS
libsampling-debug: libsampling

libsampling:
	$(CC) $(PAPI_INCLUDE_FLAGS) $(PP_FLAGS) -I./src $(INSTRO_FLAGS) $(OPT_FLAGS) $(CFLAGS) -o lib/$(LIBNAME).so $(SRC) -L./lib $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libhash: LDFLAGS+=-lhash
libhash:
	g++ -fPIC -shared $(OPT_FLAGS) -std=c++0x -Wall src/cpp/hash.cpp -o lib/libhash.so

libshadowstack: PP_FLAGS+=-DNO_PAPI_DRIVER
libshadowstack: LIBNAME=libshadowstack
libshadowstack: libhash libsampling

#measure: PP_FLAGS+=-DPRINT_FUNCTIONS
measure: timing libhash libshadowstack
	$(CC) -std=gnu99 $(OPT_FLAGS) -I./src -I$(LIBMONITOR_BASE)/include overhead/overhead-driver.c -L./lib -ltiming -lhash -lshadowstack $(LIBUNWIND_FLAGS) $(LDFLAGS) -o overhead.exe
	python3 py/gen.py overhead.exe
#	LD_PRELOAD="./lib/libshadowstack.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./overhead.exe

measure-cyg-nocache: PP_FLAGS:=-DNO_UNW_CACHE
measure-cyg-nocache: measure-cyg

measure-cyg: PP_FLAGS+=-DNO_CPP_LIB -DNO_PAPI_DRIVER -DNO_CYG_PROF -DNO_MONITOR -DIGNORE_PAPI_CONTEXT
measure-cyg: LDFLAGS:=-L./lib -ltiming_papi $(LD_FLAGS)
measure-cyg: SRC+= overhead/overhead-cyg_profile.c
measure-cyg: LIBNAME=liboverhead
measure-cyg: target timing_papi libsampling
#	LD_PRELOAD="./lib/liboverhead.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe &> out

measure-papi: PP_FLAGS+=-DIGNORE_PAPI_CONTEXT
measure-papi: timing_papi libhash libshadowstack
	$(CC) -std=gnu99 $(PP_FLAGS) $(OPT_FLAGS) -I./src overhead/overhead-driver-no-papi.c -L./lib $(LDFLAGS) -ltiming_papi -lshadowstack $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) -o overhead.papi.exe
	python3 py/gen.py overhead.papi.exe

measure-papi-link: PP_FLAGS+=-DNO_PAPI_DRIVER
measure-papi-link: PP_FLAGS+=-DIGNORE_PAPI_CONTEXT
measure-papi-link: LDFLAGS+=-ltiming_papi
measure-papi-link: libhash
	$(CC) -std=gnu99 $(PP_FLAGS) $(OPT_FLAGS) -I./src -I./overhead  overhead/overhead-driver-no-papi.c $(SRC) -L./lib -ltiming_papi -lhash $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) -o overhead.papi.exe
	python3 py/gen.py overhead.papi.exe
	
count-calls: target
	$(CC) $(OPT_FLAGS) $(CFLAGS) overhead/count-calls.c -o lib/libcount.so $(LIBMONITOR_FLAGS)
	LD_PRELOAD="./lib/libcount.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
timing:
	$(CC) -O3 $(CFLAGS) libtiming/timing.c -o lib/libtiming.so -lrt
	
timing_papi:
	$(CC) -O2 $(CFLAGS) libtiming_papi/timing.c -o lib/libtiming_papi.so -lrt -lpapi

libemptypushpop:
	$(CC) -O3 $(CFLAGS) emptypushpop/emptypushpop.c -o lib/libemptypushpop.so
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack
	touch nm_file regions_file
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)
	./test_stack.exe

sampling: libsampling-debug
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 target.c -o target.exe
	python3 py/gen.py target.exe
	LD_PRELOAD="./lib/libsampling.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
#target: EXCLUDE=-finstrument-functions-exclude-function-list=main,foo
target:
	$(CC) -g -O0 -std=gnu99 -finstrument-functions $(EXCLUDE) overhead/target.c -o target.exe
	$(CC) -g -O0 -std=gnu99 -finstrument-functions $(EXCLUDE) overhead/target-bigframe.c -o target-bigframe.exe
	
.PHONY : clean
clean:
	rm -f lib/*.so
	rm -f *.exe
	rm -f nm_file regions_file map_file stack_file
