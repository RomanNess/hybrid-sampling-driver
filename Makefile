
CC=gcc

CFLAGS=-g -O3 -fPIC -Wall
LDFLAGS=-lc $(PAPI_LD_FLAGS) -lpapi -lpthread

INSTRO_FLAGS=-DWITH_MAX_SIZE

# Our empty implementation of the cyg_profile_func_enter/exit interface
libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop/emptypushpop.c


# Sampling tool
objects = stack.o driver.o

sampling-tool: $(objects)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(INSTRO_FLAGS) $(CFLAGS) -shared -o sampling-tool.so $(objects) $(LDFLAGS)

$(objects): %.o: src/%.c
	$(CC) $(PAPI_INCLUDE_FLAGS) -c $(CFLAGS) $< -o $@

# Shadow stack as a library to link against GCC instrumented binaries
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -O3 -fPIC -shared -o libshadowstack-fast.so src/stack.c src/driver.c $(LDFLAGS)

libshadowstack-debug:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -DDEBUG -g -O0 -fPIC -shared -o libshadowstack-debug.so src/stack.c src/driver.c $(LDFLAGS)


# Sampling library (no automatic initialization) 
sampling-as-lib:
	$(CC) $(PAPI_INCLUDE_FLAGS) -g -DSAMPLING_AS_LIB -I. -fPIC -O0 -shared -o libsampling-debug.so src/stack.c src/driver.c $(LDFLAGS)

testStack: sampling-as-lib
	$(CC) $(PAPI_INCLUDE_FLAGS) -g -I./src -O0 -o test_stack.exe test.c -L. -lsampling-debug $(LDFLAGS)


.PHONY : target
target: libshadowstack-fast
	$(CC) -fopenmp -finstrument-functions -g target.c libshadowstack-fast.so -o target.exe -std=c99
	
.PHONY : sampling
sampling: sampling-tool
	$(CC) -fopenmp -finstrument-functions -g target.c sampling-tool.so -o target.exe -std=c99
	
.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
	rm -f *.exe
