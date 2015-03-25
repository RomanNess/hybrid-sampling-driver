# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)
$(call check-var-defined,LIBUNWIND_BASE)

CC=gcc

SRC=src/stack.c src/driver.c src/unwinding.c

CFLAGS=-fPIC -shared -Wall -std=gnu99
LDFLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -pthread
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread
LIBUNWIND_FLAGS=-I$(LIBUNWIND_BASE)/include -L$(LIBUNWIND_BASE)/lib -lunwind-x86_64 -lunwind

INSTRO_FLAGS=-DWITH_MAX_SIZE


libsampling: libhash
	$(CC) $(PAPI_INCLUDE_FLAGS) -DUSE_CPP_LIB -I./src $(INSTRO_FLAGS) -g -O3 $(CFLAGS) -o lib/libsampling.so $(SRC) -L./lib -lhash $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libhash:
	g++ -fPIC -shared -std=c++0x src/cpp/hash.cpp -o lib/libhash.so

# Shadow stack ONLY as a library to link against GCC instrumented binaries
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -O3 $(CFLAGS) 			-o lib/libshadowstack.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)
libshadowstack-debug:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -DDEBUG -g -O0 $(CFLAGS) -o lib/libshadowstack.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)

measure: timing
	$(CC) -std=gnu99 -O3 -I. overhead/overhead-driver.c -ltiming -L./lib $(LDFLAGS) $(LIBUNWIND_FLAGS) -o overhead.exe

timing:
	$(CC) -std=gnu99 -O3 $(CFLAGS) -o lib/libtiming.so libtiming/timing.c -lrt

libemptypushpop:
	$(CC) -O3 -fPIC -shared -o lib/libemptypushpop.so emptypushpop/emptypushpop.c
	

### Targets & Tests
testStack: SRC=src/stack.c src/driver.c
testStack:	libshadowstack-fast
	$(CC) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L./lib -lshadowstack $(LIBMONITOR_FLAGS)

sampling: libsampling
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 target.c -o target.exe
	python3 py/gen.py target.exe
	LD_PRELOAD="./lib/libsampling.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
.PHONY : clean
clean:
	rm -f lib/*.so
	rm -f *.exe
	rm -f nm_file regions_file map_file stack_file
