#SAMPLING_DRIVER_SRC=PAPI_driver_test.c
#SAMPLING_DRIVER_OBJ=$(SAMPLING_DRIVER_SRC:.cpp=.o)
#SAMPLING_DRIVER=PAPI_driver_test.so

CC="gcc"


$(SAMPLING_INFRASTRUCTURE_BENCHMARK): $(SAMPLING_INFRASTRUCTURE_SRC) 
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DINTERNAL_BENCHMARK -o $@ $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi -lrt

libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop.c

libshadowstack:
	$(CC) --version
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DSHADOWSTACK_ONLY -o libshadowstack.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi

libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DSHADOWSTACK_ONLY -o libshadowstack-f.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi

test:
	$(CC) -g -O0 -DSTACK_IS_UNDER_TEST $(SAMPLING_INFRASTRUCTURE_SRC)


CFLAGS=-g -O3 -fPIC
LDFLAGS=-lc

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

sampling_tool: $(OBJECTS)
	$(CC) $(PAPI_INCLUDE_FLAGS) $(CFLAGS) -shared -o sampling_tool.so $(OBJECTS) $(LDFLAGS) $(PAPI_LD_FLAGS)

.PHONY : clean
clean:
	rm -f *.so
	rm -f *.o
