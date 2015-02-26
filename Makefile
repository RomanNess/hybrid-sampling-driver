# check if variables are defined, as of: http://stackoverflow.com/a/22540516
check-var-defined = $(if $(strip $($1)),,$(error "$1" is not defined))
$(call check-var-defined,LIBMONITOR_BASE)
$(call check-var-defined,LIBUNWIND_BASE)

CC=gcc

SRC=src/stack.c src/driver.c src/unwinding.c

CFLAGS=-fPIC -shared -Wall -std=gnu99
LDFLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -lpthread
LIBMONITOR_FLAGS=-I$(LIBMONITOR_BASE)/include -L$(LIBMONITOR_BASE)/lib -lmonitor -pthread
LIBUNWIND_FLAGS=-I$(LIBUNWIND_BASE)/include -L$(LIBUNWIND_BASE)/lib -lunwind-x86_64 -lunwind

INSTRO_FLAGS=-DWITH_MAX_SIZE


sampling-tool: $(objects)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(INSTRO_FLAGS) -g -O3 $(CFLAGS) -o sampling-tool.so $(SRC) $(LIBUNWIND_FLAGS) $(LIBMONITOR_FLAGS) $(LDFLAGS)

# Shadow stack ONLY as a library to link against GCC instrumented binaries
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -O3 $(CFLAGS) -o libshadowstack.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)
libshadowstack-debug:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -DDEBUG -g -O0 $(CFLAGS) -o libshadowstack.so $(SRC) $(LIBMONITOR_FLAGS) $(LDFLAGS)

libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop/emptypushpop.c

### Targets & Tests
testStack: libshadowstack-fast
	$(CC) $(PAPI_INCLUDE_FLAGS) -g -std=gnu99 -I./src -O0 -o test_stack.exe test.c -L. -lshadowstack -L$(LIBMONITOR_BASE)/lib -lmonitor $(LDFLAGS)

target: libshadowstack-fast
	$(CC) -fopenmp -finstrument-functions -g  -std=gnu99 target.c libshadowstack.so -o target.exe
	LD_PRELOAD="sampling-tool.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe

sampling: sampling-tool
	$(CC) -fopenmp -finstrument-functions -g  -std=gnu99 target.c -o target.exe
	LD_PRELOAD="sampling-tool.so $(LIBMONITOR_BASE)/lib/libmonitor.so" ./target.exe
	
sampling-lib:
	$(CC) -fopenmp -finstrument-functions -g -std=gnu99 sampling-tool.so $(LIBMONITOR_BASE)/lib/libmonitor.so target.c -o target.exe
	
.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
	rm -f *.exe
