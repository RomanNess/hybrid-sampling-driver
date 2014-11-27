#SAMPLING_DRIVER_SRC=PAPI_driver_test.c
#SAMPLING_DRIVER_OBJ=$(SAMPLING_DRIVER_SRC:.cpp=.o)
#SAMPLING_DRIVER=PAPI_driver_test.so

CC=gcc

CFLAGS=-g -O3 -fPIC -Wall
LDFLAGS=-lc

MT_FLAGS=-DUSE_THREAD_WRITE_OUT

INSTRO_FLAGS=-DWITH_MAX_SIZE

# Our empty implementation of the cyg_profile_func_enter/exit interface
libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop/emptypushpop.c


# --- Start of the sampling tool ---

objects = stack.o driver.o

sampling_tool: $(objects)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(INSTRO_FLAGS) $(CFLAGS) -shared -o sampling_tool.so $(objects) $(LDFLAGS) $(PAPI_LD_FLAGS)

$(objects): %.o: %.c
	$(CC) $(PAPI_INCLUDE_FLAGS) -c $(CFLAGS) $< -o $@

# We can build the shadow stack as a library to link against GCC instrumented binaries.
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSHADOWSTACK_ONLY -O3 -fPIC -shared -o libshadowstack-fast.so stack.c driver.c -lc $(PAPI_LD_FLAGS) -lpapi -lpthread

libshadowstack-debug:
	$(CC) $(PAPI_INCLUDE_FLAGS) -DDEBUG -DSHADOWSTACK_ONLY -g -O0 -fPIC -shared -o libshadowstack-debug.so stack.c driver.c -lc $(PAPI_LD_FLAGS) -lpapi -lpthread



sampling-as-lib:
	$(CC) -g -DSAMPLING_AS_LIB $(PAPI_INCLUDE_FLAGS) -I. -fPIC -O0 -shared -o libsampling-debug.so stack.c driver.c -lc $(PAPI_LD_FLAGS) -lpthread -lpapi


testStack: sampling-as-lib
	$(CC) -g  -I. $(PAPI_INCLUDE_FLAGS)  -O0 -o test_stack.exe test.c   $(PAPI_LD_FLAGS) -L. -lsampling-debug -lpapi -lpthread


.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
	rm -f test_stack.exe
