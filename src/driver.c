#define _GNU_SOURCE	// for REG_RIP
#include "driver.h"

#ifdef META_BENCHMARK
#include "libtiming/timing.h"
#endif

#include <assert.h>

#include <stdio.h>
#include <sys/time.h>	// itimer
#include <signal.h>		// signal
#include <ucontext.h>	// greps[REG_RIP]

int initialized = 0;

struct Stack **_multithreadStack = 0;
struct SampleEvent *_flushToDiskBuffer = 0;
struct StackEvent *_innerBuffer = 0;
unsigned long _innerBufferSize = 0;

long samplesTaken = 0;
long samplesInDriverRegion = 0;
unsigned int numberOfBufferElements = 0;
long overflowCountForSamples = 2500000;

#ifndef NO_PAPI_DRIVER
__thread int EventSet = PAPI_NULL;
#endif

#ifndef NO_ITIMER_DRIVER
static struct itimerval itimer;
static int itimerLock = 0;
static long samplesOmitted = 0;
#endif

void initBuffer() {
	if (_flushToDiskBuffer != 0) {
		return;
	}

	size_t allocSize = WRITE_BUFFER_SIZE * 2 * sizeof(struct SampleEvent);
	_flushToDiskBuffer = (struct SampleEvent *) malloc(allocSize);
	size_t innerAllocSize = WRITE_BUFFER_SIZE * 5 * sizeof(struct StackEvent);
	_innerBuffer = (struct StackEvent *) malloc(innerAllocSize);


	if (_flushToDiskBuffer == 0) {
		errx(1, "Could not allocate a write-out buffer with size: %lu bytes", allocSize);
	}
	if (_innerBuffer == 0) {
		errx(1, "Could not allocate _innerBuffer");
	}
	printf("Allocated %lu bytes for buffer.\n", allocSize);

}

void finiBuffer() {
	if (_flushToDiskBuffer != 0) {
		free(_flushToDiskBuffer);
		free(_innerBuffer);
	}
}

/*
 * This is the function to be called when PAPI interrupts and a sample is taken by our handler.
 * It saves the state of the shadow stack and the PAPI instruction counter address to a
 * SampleEvent object and puts it in the buffer.
 */
void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer) {

	if (stack == 0 || buffer == 0) {
		errx(-5, "Error: stack or buffer was NULL. Exiting.");
	}

	buffer[numberOfBufferElements].thread = threadId;
	buffer[numberOfBufferElements].sampleNumber = samplesTaken;
	if (stack->_size == 0) {
#if DEBBUG
		fprintf(stderr, "FlushStackToBuffer: %li with stack size == 0\n", samplesTaken);
#endif
		numberOfBufferElements++;
		return;
	}

	buffer[numberOfBufferElements].stackEvents = &_innerBuffer[_innerBufferSize];
	_innerBufferSize += stack->_size;

	if (buffer[numberOfBufferElements].stackEvents == 0) {
		errx(-7, "Error creating buffer[bufferElements].stackEvents buffer");
	}

	for (int i = 0; i < stack->_size; i++) {
		buffer[numberOfBufferElements].stackEvents[i] = stack->_elements[i];
	}
	buffer[numberOfBufferElements].numStackEvents = stack->_size;
}

/*
 * Flushes the SampleEvents to a file
 * ATTENTION this produces LOTS OF DATA
 */
void flushBufferToFile(struct SampleEvent *buffer) {
	fprintf(stdout, "Starting to write out\n");
	FILE *fp = fopen("stack_file", "a+");

	if (fp) {
		// write all buffered elements to a file
		for (int i = 0; i < numberOfBufferElements; i++) {

			const struct StackEvent *stackEvents = buffer[i].stackEvents;
			fprintf(fp, "Sample: %lu\nAddress: %lx\n", buffer[i].sampleNumber, buffer[i].icAddress);
			fprintf(fp, "ShadowStack size: %i, Unwind size: %i\n", buffer[i].numStackEvents, buffer[i].numUnwindEvents);

			for (int j = 0; j < buffer[i].numStackEvents; j++) {
				unsigned long address  = stackEvents[j].identifier;
				const char* name = getName(address);
				fprintf(fp, "Thread: %i in Function: %lx (%s)\n", buffer[i].thread, address, name);
			}

			const struct StackEvent* unwindEvents = buffer[i].unwindEvents;
			for (int j = 0; j < buffer[i].numUnwindEvents; j++) {
				unsigned long address = unwindEvents[j].identifier;
				const char* name = getName(address);
				fprintf(fp, "Unwind: %lx (%s)\n", address, name);
			}

			fprintf(fp, "\n");
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
	abstractHandler((unsigned long) address, context);
#endif //NO_PAPI_HANDLER
}

#ifndef NO_ITIMER_DRIVER
int signalHandler(int sig, siginfo_t* siginfo, void* context) {

	if (itimerLock) {
		samplesOmitted++;
		printf("#");
		return 0;
	}
	itimerLock = 1;

	samplesTaken++;

#ifndef NO_PAPI_HANDLER
	ucontext_t* uc = (ucontext_t*) context;
	unsigned long address = uc->uc_mcontext.gregs[REG_RIP];
	abstractHandler(address, context);
#endif //NO_PAPI_HANDLER

	itimerLock = 0;

	return 0;
}
#endif // NO_ITIMER_DRIVER

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

#ifndef NO_PAPI_DRIVER
	initPapiSamplingDriver();
#endif	//  NO_PAPI_DRIVER
#ifndef NO_ITIMER_DRIVER
	initItimerSamplingDriver();
#endif

}

void initPapiSamplingDriver() {
	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}
	if ((retval = PAPI_thread_init(getThreadId)) != PAPI_OK) {
		errx(retval, "PAPI_thread_init failed with %i", retval);
	}

	registerThreadForPAPI();
	printf("PAPI sampling driver enabled. Sampling every %li micros.\n", overflowCountForSamples/2500);
}

#ifndef NO_ITIMER_DRIVER
void initItimerSamplingDriver() {
	long int micros=overflowCountForSamples/2500;


	itimer.it_value.tv_sec  = 0;
	itimer.it_value.tv_usec = micros;
	itimer.it_interval = itimer.it_value;

	if (setitimer(ITIMER_REAL, &itimer, NULL) != 0) {
		printf("ERROR: setitimer() failed\n");
	}
	if (monitor_sigaction(SIGALRM, &signalHandler, 0, NULL) != 0) {
		printf("ERROR: monitor_sigacton() failed.\n");
	}
	printf("Initialized itimer driver. Sampling every %li micros\n", micros);
}
int emptyHandler(int sig, siginfo_t* siginfo, void* context) {
	return 0;
}
void finiItimerSamplingDriver() {
	if (monitor_sigaction(SIGALRM, &emptyHandler, 0, NULL) != 0) {
		printf("ERROR: monitor_sigaction() failed in fini.\n");
	}
}
#endif

#ifndef NO_PAPI_DRIVER

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

void finiPapiSamplingDriver() {

	long long instructionCounter;
	PAPI_stop(EventSet, &instructionCounter);

	printf("PAPI sampling driver disabled\n");

#ifdef WITH_MAX_SIZE
	printf("The max stack size reached was: %u\n", stackMaxSize);
#endif
}

#endif	// NO_PAPI_DRIVER

#ifndef NO_INIT

#ifdef MONITOR_INIT
void *monitor_init_process(int *argc, char **argv, void *data) {
	printf("#### Init driver with LIBMONITOR - Pid is %i #### \n", getpid());
#else	//MONITOR_INIT
__attribute__((constructor))
void *_init_process(int *argc, char **argv, void *data) {
	printf("#### Init driver with __attribute__((constructor)) - Pid is %i #### \n", getpid());
#endif	//MONITOR_INIT

	readEnv();
	assingContinuousThreadId();
	initShadowStack();
	
	_myStack = _multithreadStack[threadId]; 

#ifndef NO_CPP_LIB
	parseFunctions("nm_file");
	parseRegions("regions_file", &targetRegionStart, &targetRegionEnd, &mainRegionStart, &mainRegionEnd);
//	dump();
	dumpMemoryMapping(&driverRegionStart, &driverRegionEnd);
#else
	printf("NO_CPP_LIB\n");
#endif

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
#ifndef NO_ITIMER_DRIVER
	finiItimerSamplingDriver();
#endif
#endif // NO_SAMPLING

	///XXX
	fprintf(stderr, "monitor_fini_process\n");

	printf("%u elements in buffer\n", numberOfBufferElements);
	flushBufferToFile(_flushToDiskBuffer);
	finiBuffer();

#ifdef META_BENCHMARK
	printResults("target");
#endif

#ifndef NO_SAMPLING
	printf("%li samples taken. %li in driver regions.\n", samplesTaken, samplesInDriverRegion);
#ifndef NO_ITIMER_DRIVER
	printf("%li overlapping samples omitted.\n", samplesOmitted);
#endif
#endif // NO_SAMPLING

	assert(_multithreadStack[threadId]->_size==0);
}

void *monitor_init_thread(int tid, void *data) {

	///XXX
	fprintf(stderr, "monitor_init_thread\n");

#ifndef NO_PAPI_DRIVER
	PAPI_register_thread();
#endif

	assingContinuousThreadId();
	_myStack = _multithreadStack[threadId];

#ifndef NO_PAPI_DRIVER
	registerThreadForPAPI();	// PAPI is registered per thread
#endif

	return NULL;
}

void monitor_fini_thread(void* data) {

	///XXX
	fprintf(stderr, "monitor_fini_thread\n");

	finiSingleStack(_multithreadStack[threadId]);	// RN: or just reset the stack?
}

#endif	// NO_INIT
