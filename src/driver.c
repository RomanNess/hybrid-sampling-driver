#include "driver.h"

struct Stack **_multithreadStack = 0;
struct SampleEvent *_flushToDiskBuffer = 0;
long int sampleCount = 0;
unsigned int numberOfBufferElements = 0;
long overflowCountForSamples = 2600000;

#ifndef NO_PAPI_DRIVER
__thread int EventSet = PAPI_NULL;
#endif

void initBuffer() {
	if (_flushToDiskBuffer != 0) {
		return;
	}

	size_t allocSize = WRITE_BUFFER_SIZE * 2 * sizeof(struct SampleEvent);
	_flushToDiskBuffer = (struct SampleEvent *) malloc(allocSize);

	if (_flushToDiskBuffer == 0) {
		errx(1, "Could not allocate a write-out buffer with size: %lu bytes", allocSize);
	}
	printf("Allocated %lu bytes for buffer.\n", allocSize);

}

void finiBuffer() {
	if (_flushToDiskBuffer != 0) {
		for (int i = 0; i < numberOfBufferElements; i++) {
			free(_flushToDiskBuffer[i].stackEvents); // Correct?
		}
		free(_flushToDiskBuffer);
	}
}

/*
 * XXX JP: The function needs some attention.
 *
 * This is the function to be called when PAPI interrupts and a sample is taken by our handler.
 * It saves the state of the shadow stack and the PAPI instruction counter address to a
 * SampleEvent object and puts it in the buffer.
 */
void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer) {

	if (stack == 0 || buffer == 0) {
		errx(-5, "Error: stack or buffer was NULL. Exiting.");
	}

	buffer[numberOfBufferElements].thread = threadId;
	buffer[numberOfBufferElements].sampleNumber = sampleCount;
	if (stack->_size == 0) {
#if DEBBUG
		fprintf(stderr, "FlushStackToBuffer: %li with stack size == 0\n", sampleCount);
#endif
		numberOfBufferElements++;
		return;
	}

	buffer[numberOfBufferElements].stackEvents = (struct StackEvent *) malloc(stack->_size * sizeof(struct StackEvent));
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
 * XXX: ATTENTION this produces LOTS OF DATA
 * For example we could use /scratch to output lots of GB ...
 * Dataformat:
 * [Sample ID] [icAddress] [stackelems]
 *
 * XXX 2014-05-12 JP: Move the whole file operation thing to a different file?
 * XXX 2014-05-12 JP: Use the cube file writer lib to output cubex files?
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
				fprintf(fp, "Thread: %i in Function: %lx\n", buffer[i].thread, stackEvents[j].identifier);
			}

			const struct StackEvent* unwindEvents = buffer[i].unwindEvents;
			for (int j = 0; j < buffer[i].numUnwindEvents; j++) {


				fprintf(fp, "Unwind: %lx\n", unwindEvents[j].identifier);
			}

			fprintf(fp, "\n");
			free((struct StackEvent *) stackEvents);
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
	sampleCount++;

	// This is where the work happens
	flushStackToBuffer(_multithreadStack[threadId], _flushToDiskBuffer);

	long startAddress = doUnwind((unsigned long) address, context, &_flushToDiskBuffer[numberOfBufferElements]);

	_flushToDiskBuffer[numberOfBufferElements].icAddress = startAddress;

	numberOfBufferElements++;
}

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

void initSamplingDriver() {

	/* read environment variable to control the sampling driver */
	char *instroFreqVariable = getenv("INSTRO_SAMPLE_FREQ");
	if (instroFreqVariable != NULL) {
		printf("Using sample frequency set with INSTRO_SAMPLE_FREQ = %s\n", instroFreqVariable);
		overflowCountForSamples = atoi(instroFreqVariable);
	} else {
		printf("INSTRO_SAMPLE_FREQ not set, using a sample each 2600000 cycles\n");
	}

	initBuffer();

	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}
	if ((retval = PAPI_thread_init(getThreadId)) != PAPI_OK) {
		errx(retval, "PAPI_thread_init failed with %i", retval);
	}

	registerThreadForPAPI();

	printf("Sampling Driver Enabled\n");
}

void finishSamplingDriver() {

	long long instructionCounter;
	PAPI_stop(EventSet, &instructionCounter);

	printf("%li samples taken\n", sampleCount);
	printf("%u elements in buffer\n", numberOfBufferElements);

	flushBufferToFile(_flushToDiskBuffer);

	finiBuffer();
	printf("Sampling Driver Disabled\n");

#ifdef WITH_MAX_SIZE
	printf("The max stack size reached was: %u\n", stackMaxSize);
#endif
}

#endif	// NO_PAPI_DRIVER

void *monitor_init_process(int *argc, char **argv, void *data) {

	printf("#### init sampling driver - Pid is %i #### \n", getpid());

	readEnv();
	assingContinuousThreadId();
	initShadowStack();

#ifdef USE_CPP_LIB
	parseFunctions("nm_file");
	parseRegions("regions_file", &regionStart, &regionEnd);
//	dump();
	dumpMemoryMapping();
#endif

#ifndef NO_PAPI_DRIVER
	initSamplingDriver();
#endif

	return NULL;
}

void monitor_fini_process(int how, void* data) {
#ifndef NO_PAPI_DRIVER
	finishSamplingDriver();
#endif
}

void *monitor_init_thread(int tid, void *data) {

	PAPI_register_thread();
	assingContinuousThreadId();

#ifndef NO_PAPI_DRIVER
	registerThreadForPAPI();	// PAPI is registered per thread
#endif

	return NULL;
}

void monitor_fini_thread(void* data) {
	finiSingleStack(_multithreadStack[threadId]);	// RN: or just reset the stack?
}

