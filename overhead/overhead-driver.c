
#include <papi.h>

#include <stdio.h>
#include <err.h>

long overflowCountForSamples = 2600000;
long sampleCount = 0;

int EventSet = PAPI_NULL;


void handler(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;
	//printf("  sample  \n");
}

void initPAPI() {

	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}

	if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) {
		errx(retval, "PAPI_create_eventset failed with %i", retval);
	}
	if ((retval = PAPI_add_event(EventSet, PAPI_TOT_CYC)) != PAPI_OK) {
		errx(retval, "PAPI_create_eventset failed with %i", retval);
	}
	if ((retval = PAPI_overflow(EventSet, PAPI_TOT_CYC, overflowCountForSamples, 0, handler)) != PAPI_OK) {
		errx(retval, "PAPI_overflow failed with %i", retval);
	}
	if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
		errx(retval, "PAPI_start failed with %i", retval);
	}

}

void finiPAPI() {
	long long instructionCounter;
	PAPI_stop(EventSet, &instructionCounter);

	printf("%li samples taken\n", sampleCount);
}

int main() {

	initPAPI();

	float f = 42.0f;
	for (int i=1; i< 10000000; i++) {
		f = f + 1/i;
	}

	finiPAPI();

	return 0;
}
