#SAMPLING_DRIVER_SRC=PAPI_driver_test.c
#SAMPLING_DRIVER_OBJ=$(SAMPLING_DRIVER_SRC:.cpp=.o)
#SAMPLING_DRIVER=PAPI_driver_test.so

CC="gcc"

CFLAGS=-g -O3 -fPIC
LDFLAGS=-lc

INSTRO_FLAGS=-DWITH_MAX_SIZE

# Our empty implementation of the cyg_profile_func_enter/exit interface
libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop.c


# --- Start of the sampling tool ---

objects = driver.o stack.o

sampling_tool: $(objects)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(INSTRO_FLAGS) $(CFLAGS) -shared -o sampling_tool.so $(objects) $(LDFLAGS) $(PAPI_LD_FLAGS)

$(objects): %.o: %.c
	$(CC) $(PAPI_INCLUDE_FLAGS) -c $(CFLAGS) $< -o $@

# We can build the shadow stack as a library to link against GCC instrumented binaries.
libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -o libshadowstack-fast.so stack.c -lc $(PAPI_LD_FLAGS) -lpapi -lpthread




.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
