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
PP_FLAGS=-DUSE_CPP_LIB

libsampling-debug: PP_FLAGS+=-DPRINT_FUNCTIONS
libsampling-debug: libsampling

libsampling: libhash
	$(CC) $(PAPI_INCLUDE_FLAGS) $(PP_FLAGS) -I./src $(INSTRO_FLAGS) $(OPT_FLAGS) $(CFLAGS) -o lib/$(LIBNAME).so $(SRC) -L./lib -lhash $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libhash:
	g++ -fPIC -shared $(OPT_FLAGS) -std=c++0x -Wall src/cpp/hash.cpp -o lib/libhash.so

libshadowstack-fast: PP_FLAGS+=-DNO_PAPI_DRIVER
libshadowstack-fast: LIBNAME=libshadowstack
libshadowstack-fast: libsampling

libshadowstack-debug: OPT_FLAGS=-g -O0
libshadowstack-debug: libshadowstack-fast

#measure: PP_FLAGS+=-DPRINT_FUNCTIONS
measure: timing libhash libshadowstack-fast
	$(CC) -std=gnu99 -O3 -I./src -I$(LIBMONITOR_BASE)/include overhead/overhead-driver.c -L./lib -ltiming -lhash -lshadowstack $(LDFLAGS) $(LIBUNWIND_FLAGS) -o overhead.exe
	python3 py/gen.py overhead.exe

measure-papi: PP_FLAGS+=-DIGNORE_PAPI_CONTEXT
measure-papi: libshadowstack-fast
	$(CC) -std=gnu99 $(PP_FLAGS) $(OPT_FLAGS) -I./src overhead/overhead-driver-no-papi.c -L./lib -ltiming_papi -lshadowstack $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) -o overhead.papi.exe
	python3 py/gen.py overhead.papi.exe

measure-papi-link: PP_FLAGS+=-DNO_PAPI_DRIVER
measure-papi-link: PP_FLAGS+=-DIGNORE_PAPI_CONTEXT
measure-papi-link: LDFLAGS+=-ltiming_papi
measure-papi-link: libhash
	$(CC) -std=gnu99 $(PP_FLAGS) $(OPT_FLAGS) -I./src -I./overhead  overhead/overhead-driver-no-papi.c $(SRC) -L./lib -ltiming_papi -lhash $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) -o overhead.papi.exe
	python3 py/gen.py overhead.papi.exe
	
timing:
	$(CC) -std=gnu99 -O3 $(CFLAGS) -o lib/libtiming.so libtiming/timing.c -lrt

libemptypushpop:
	$(CC) -O3 -fPIC -shared -o lib/libemptypushpop.so emptypushpop/emptypushpop.c
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack-fast
	touch nm_file regions_file
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)

sampling: libsampling-debug
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 target.c -o target.exe
	python3 py/gen.py target.exe
	LD_PRELOAD="./lib/libsampling.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
.PHONY : clean
clean:
	rm -f lib/*.so
	rm -f *.exe
	rm -f nm_file regions_file map_file stack_file
