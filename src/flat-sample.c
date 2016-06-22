#define _GNU_SOURCE	// for REG_RIP

#define META_BENCHMARK
#ifdef META_BENCHMARK
#include "libtiming/timing.h"
#endif

#include <assert.h>

#include <stdio.h>
#include <sys/time.h>	// itimer
#include <signal.h>		// signal
#include <ucontext.h>	// greps[REG_RIP]

#include <err.h>	// for errx
#include <unistd.h>	// for getpid
#include <stdlib.h>

#include <papi.h>
#include <monitor.h>

int initialized = 0;

unsigned long long* buffer = 0;

long samplesTaken = 0;
unsigned int numberOfBufferElements = 0;
long overflowCountForSamples = 2500000;

int EventSet = PAPI_NULL;

void initBuffer() {
	if (buffer != 0) {
		return;
	}

	long unsigned allocSize = 50*1000*1000 * sizeof(unsigned long*);
	buffer = (unsigned long long*) malloc(allocSize);

	if (buffer == 0) {
		errx(1, "Could not allocate a write-out buffer with size: %lu bytes", allocSize);
	}
	printf("Allocated %lu bytes for buffer.\n", allocSize);

}

void finiBuffer() {
	if (buffer != 0) {
		free(buffer);
	}
}

/*
 * Flushes the SampleEvents to a file
 * ATTENTION this produces LOTS OF DATA
 */
void flushBufferToFile() {
	fprintf(stdout, "Starting to write out\n");
	FILE *fp = fopen("flat_profile", "a+");

	if (fp) {
		// write all buffered elements to a file
		for (int i = 0; i < numberOfBufferElements; i++) {

			fprintf(fp, "%llx\n", buffer[i]);
		}
		numberOfBufferElements = 0;

		int fErr = fclose(fp);
		fprintf(stdout, "fclose exited with %i\n", fErr);
	} else {
		fprintf(stderr, "Could not open file for writing.\n");
	}
}

/* PAPI Sampling handler */
void handler(int EventSet, void* address, long long overflow_vector, void* context) {
	samplesTaken++;

#ifndef NO_PAPI_HANDLER
	buffer[numberOfBufferElements++] = (unsigned long long) address;
#endif //NO_PAPI_HANDLER
}


/*
 * REGISTER SAMPLING DRIVER SEGMENT
 */

void registerThreadForPAPI() {
	int retval;
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

void initPapiSamplingDriver() {
	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}
	if ((retval = PAPI_thread_init(0)) != PAPI_OK) {
		errx(retval, "PAPI_thread_init failed with %i", retval);
	}

	registerThreadForPAPI();
	printf("PAPI sampling driver enabled. Sampling every %li micros.\n", overflowCountForSamples/2500);
}

void initSamplingDriver() {

	/* read environment variable to control the sampling driver */
	char *instroFreqVariable = getenv("INSTRO_SAMPLE_FREQ");
	if (instroFreqVariable != NULL) {
		printf("Using sample frequency set with INSTRO_SAMPLE_FREQ = %s\n", instroFreqVariable);
		overflowCountForSamples = atoi(instroFreqVariable);
	} else {
		printf("INSTRO_SAMPLE_FREQ not set, using a sample each %lu cycles\n", overflowCountForSamples);
	}

	initBuffer();

	initPapiSamplingDriver();

}


void finiPapiSamplingDriver() {

	long long instructionCounter;
	PAPI_stop(EventSet, &instructionCounter);

	printf("PAPI sampling driver disabled\n");

#ifdef WITH_MAX_SIZE
	printf("The max stack size reached was: %u\n", stackMaxSize);
#endif
}

#ifdef MONITOR_INIT
void *monitor_init_process(int *argc, char **argv, void *data) {
	printf("#### Init flat driver with LIBMONITOR - Pid is %i #### \n", getpid());
#else	//MONITOR_INIT
__attribute__((constructor))
void *_init_process(int *argc, char **argv, void *data) {
	printf("#### Init flat driver with __attribute__((constructor)) - Pid is %i #### \n", getpid());
#endif	//MONITOR_INIT

#ifndef NO_SAMPLING
	initSamplingDriver();
#endif
	initialized = 1;

#ifdef META_BENCHMARK
	initMeasurement();
	printResultsHeader();
	// warmup
	startMeasurement();
	stopMeasurement();
	startMeasurement();
	stopMeasurement();
	startMeasurement();
	stopMeasurement();

	startMeasurement();

#endif

	return NULL;
}

#if MONITOR_INIT
void monitor_fini_process(int how, void* data) {
#else //MONITOR_INIT
__attribute__((destructor))
void _fini_process(int how, void* data) {
#endif	//MONITOR_INIT

#ifdef META_BENCHMARK
	stopMeasurement();
#endif

#ifndef NO_SAMPLING
#ifndef NO_PAPI_DRIVER
	finiPapiSamplingDriver();
#endif
#endif // NO_SAMPLING


#ifdef META_BENCHMARK
	printResults("target");
#endif

	printf("%u elements in buffer\n", numberOfBufferElements);
	flushBufferToFile();
	finiBuffer();

#ifndef NO_SAMPLING
	printf("%li samples taken.\n", samplesTaken);
#endif // NO_SAMPLING

}


