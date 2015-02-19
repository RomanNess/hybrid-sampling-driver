# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)

CC=gcc

SRC=src/stack.c src/driver.c

CFLAGS=-fPIC -shared -Wall
LDFLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -lpthread
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread

INSTRO_FLAGS=-DWITH_MAX_SIZE


sampling-tool: $(objects)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(INSTRO_FLAGS) -g -O3 $(CFLAGS) -o sampling-tool.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS) -std=gnu99

# Shadow stack ONLY as a library to link against GCC instrumented binaries
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -O3 $(CFLAGS) -o libshadowstack-fast.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)
libshadowstack-debug:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -DDEBUG -g -O0 $(CFLAGS) -o libshadowstack-debug.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop/emptypushpop.c

### Targets & Tests
testStack: libshadowstack-fast
	$(CC) $(PAPI_INCLUDE_FLAGS) -g -I./src -O0 -o test_stack.exe test.c -L. -lshadowstack-fast -L$(LIBMONITOR_BASE)/lib -lmonitor $(LDFLAGS)

target: libshadowstack-fast
	$(CC) -fopenmp -finstrument-functions -g target.c libshadowstack-fast.so -o target.exe -std=gnu99

sampling: sampling-tool
	$(CC) -fopenmp -finstrument-functions -g target.c -o target.exe -std=gnu99
	LD_PRELOAD="sampling-tool.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
	rm -f *.exe
