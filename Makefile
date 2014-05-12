#SAMPLING_DRIVER_SRC=PAPI_driver_test.c
#SAMPLING_DRIVER_OBJ=$(SAMPLING_DRIVER_SRC:.cpp=.o)
#SAMPLING_DRIVER=PAPI_driver_test.so

CC="gcc"

SAMPLING_INFRASTRUCTURE_SRC=SamplingInfrastructure.c stack.c
SAMPLING_INFRASTRUCTURE_OBJ=$(SAMPLING_INFRASTRUCTURE_SRC:.c=.o)
SAMPLING_INFRASTRUCTURE=sampling_tool.so

SAMPLING_INFRASTRUCTURE_BENCHMARK=sampling_tool_benchmark.so


all: $(SAMPLING_INFRASTRUCTURE) $(SAMPLING_DRIVER) libemptypushpop libshadowstack

#$(SAMPLING_DRIVER): $(SAMPLING_DRIVER_SRC)
#	$(CC) $(PAPI_INCLUDE_FLAGS) -fPIC -shared -o $@ $(SAMPLING_DRIVER_SRC) -lc -lpapi $(PAPI_INCLUDE_FLAGS) $(PAPI_LD_FLAGS)

$(SAMPLING_INFRASTRUCTURE): $(SAMPLING_INFRASTRUCTURE_SRC) 
	$(CC) $(PAPI_INCLUDE_FLAGS) -g -O3 -fPIC -shared -o $@ $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi 
sampling-as-lib:$(SAMPLING_INFRASTRUCTURE_SRC) 
	$(CC) $(PAPI_INCLUDE_FLAGS) -DSAMPLING_AS_LIB -O3 -fPIC -shared -o libsampling.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi 


$(SAMPLING_INFRASTRUCTURE_BENCHMARK): $(SAMPLING_INFRASTRUCTURE_SRC) 
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DINTERNAL_BENCHMARK -o $@ $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi -lrt

libemptypushpop:
	$(CC) -O2 -fPIC -shared -o libemptypushpop.so emptypushpop.c

libshadowstack:
	$(CC) --version
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DSHADOWSTACK_ONLY -o libshadowstack.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi
#	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DSHADOWSTACK_ONLY -o libshadowstack-with-papi.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc -Wl,--whole-archive $(PAPI_BASE_DIR)/lib/libpapi.a -Wl,-no-whole-archive

libshadowstack-fast:
	$(CC) $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -DSHADOWSTACK_ONLY -o libshadowstack-f.so $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi

libtiming:
	$(CC) -fPIC -O2 -shared -o libtiming.so timing.c -lrt

assembly:
	$(CC) -S -g $(PAPI_INCLUDE_FLAGS) -O3 -fPIC -shared -o sampling_infrastructure_assembly.s $(SAMPLING_INFRASTRUCTURE_SRC) -lc $(PAPI_LD_FLAGS) -lpapi

shadowstack:
	$(CC) ShadowStack.c

test:
	$(CC) -g -O0 -DSTACK_IS_UNDER_TEST $(SAMPLING_INFRASTRUCTURE_SRC)


clean:
	rm -f *.so
