#include "driver.h"

#include <err.h>

void initBuffer() {
	if (_flushToDiskBuffer != 0) {
		return;
	}

	_flushToDiskBuffer = (struct SampleEvent *) malloc(WRITE_BUFFER_SIZE * sizeof(struct SampleEvent));
	if (_flushToDiskBuffer == 0) {
		errx(1, "Could not allocate a write-out buffer with size: %i", 2 * WRITE_BUFFER_SIZE);
	}

//	memset(_flushToDiskBuffer, 0, WRITE_BUFFER_SIZE * sizeof(struct SampleEvent)); // Is this line correct?
}

void deallocateWriteOutBuffer() {
	if (_flushToDiskBuffer != 0) {
		int i;
		for (i = 0; i < bufferElements; i++) {
			free(_flushToDiskBuffer[i].stackEvents); // Correct?
		}
		free(_flushToDiskBuffer);
	}
}

/*
 * This flushes the actual state of the stack to file.
 * I guess that this function at the moment is our bottleneck, since we flush buffers all the time.
 *
 * XXX JP: This function is useless I guess.
 */
void flushStackToFile(struct Stack *stack) {
	FILE *fp = fopen("myOutStack.txt", "a+");

	if (fp) {
		unsigned int cur;
		fprintf(fp, "Sample %li\n", sampleCount);
		for (cur = 0; cur < stack->_size; cur++) {
			fprintf(fp, "Thread %llu with identifier: %llu \n",
					stack->_start[cur].thread, stack->_start[cur].identifier);
		}
	}

	int fclose_return = fclose(fp);
	if (fclose_return) {
		fprintf(stderr, "Closing the output file returned an error\n");
	}
}

/*
 * XXX JP: The function needs some attention.
 *
 * This is the function to be called when PAPI interrupts and a sample is taken by our handler
 * It saves the state of the shadow stack and the PAPI instruction counter address to a
 * SampleEvent object and puts it on our buffer. XXX At least that's what it should do!
 *
 *
 */
void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer, void *icAddress) {
	if (stack == 0 || buffer == 0) {
		fprintf(stderr, "An error occured, where either stack or buffer was NULL. Exiting.\n"), exit(-5);
	}
	/*
	 if(bufferElements == WRITE_BUFFER_SIZE){
	 //              printf("A buffer flush would have been called.\n The actual stack size is %u", stack->_size);
	 struct SampleEvent sampleEvent = buffer[bufferElements];
	 buffer[bufferElements].icAddress = icAddress;
	 buffer[bufferElements].sampleNumber = sampleCount;
	 int k = 0;
	 int BORDER = (buffer[bufferElements].numStackEvents < stack->_size)?buffer[bufferElements].numStackEvents : stack->_size;
	 for(;k < BORDER; k++){
	 buffer[bufferElements].stackEvents[k] = stack->_start[k];
	 }
	 return;
	 }
	 */

	struct SampleEvent sampleEvent = buffer[bufferElements];
	buffer[bufferElements].icAddress = (long) icAddress;
	buffer[bufferElements].sampleNumber = sampleCount;
	if (stack->_size == 0) {
		fprintf(stderr, "FlushStackToBuffer: %i with stack size == 0\n", sampleCount);
		bufferElements++;
		return;
	}

	buffer[bufferElements].stackEvents = (struct StackEvent *) malloc(
			stack->_size * sizeof(struct StackEvent));

	if (buffer[bufferElements].stackEvents == 0) {
		errx(-7, "Error creating buffer[bufferElements].stackEvents buffer");
	}
	int i;
	for (i = 0; i < stack->_size; i++) {
		buffer[bufferElements].stackEvents[i] = stack->_start[i];
	}
	//      sampleEvent.numStackEvents = i;
	buffer[bufferElements].numStackEvents = i;
	fprintf(stderr, "Wrote %i stack elements\n", i);

	//      buffer[bufferElements] = *sampleEvent;
	bufferElements++;
}

/*
 * Flushes the SampleEvents to a file called "myOutStack.txt"
 * XXX: ATTENTION this produces LOTS OF DATA
 * We need to provide functionality to e.g. specify where the output has to go.
 * For example we could use /scratch to output lots of GB...
 * Dataformat:
 * [Sample ID] [icAddress] [stackelems]
 *
 * XXX 2014-05-12 JP: Move the whole file operation thing to a different file?
 * XXX 2014-05-12 JP: Use the cube file writer lib to output cubex files?
 */
void flushBufferToFile(struct SampleEvent *buffer) {
//  void *res;
	pthread_attr_init(&detachAttr);
	pthread_attr_setdetachstate(&detachAttr, PTHREAD_CREATE_DETACHED);
	int err = pthread_create(&writeThread, &detachAttr, pthread_flushBufferToFile, _flushToDiskBuffer);

	return;

	FILE *fp = fopen("myOutStack.txt", "a+");

	if (fp) {
		int i = 0;
		// write all buffered elements to a file
		for (; i < bufferElements; i++) {
			const struct StackEvent *stackEvents = buffer[i].stackEvents;
			fprintf(fp, "Sample: %lu\nAddress: %lu\n", buffer[i].sampleNumber, buffer[i].icAddress);
			int j = 0;
			for (; j < buffer[i].numStackEvents; j++) {
				fprintf(fp, "Thread: %llu in Function: %llu\n", stackEvents[j].thread, stackEvents[j].identifier);
			}
			free((struct StackEvent*) stackEvents);
		}
		bufferElements = 0;
		fclose(fp);
	} else {
		fprintf(stderr, "Could not open file for writing.\n");
	}
}

/*
 * Flushes the SampleEvents to a file called "myOutStack.txt"
 * XXX: ATTENTION this produces LOTS OF DATA
 * We need to provide functionality to e.g. specify where the output has to go.
 * For example we could use /scratch to output lots of GB...
 * Dataformat:
 * [Sample ID] [icAddress] [stackelems]
 *
 * XXX 2014-05-12 JP: Move the whole file operation thing to a different file?
 * XXX 2014-05-12 JP: Use the cube file writer lib to output cubex files?
 */
void* pthread_flushBufferToFile(void *data) {
	fprintf(stdout, "Starting to write out\n");
	struct SampleEvent *buffer = (struct SampleEvent *) data;
	FILE *fp = fopen("pthread_myOutStack.txt", "a+");

	if (fp) {
		int i = 0;
		// write all buffered elements to a file
		for (; i < bufferElements; i++) {
			const struct StackEvent *stackEvents = buffer[i].stackEvents;
			fprintf(fp, "Sample: %lu\nAddress: %lu\n", buffer[i].sampleNumber, buffer[i].icAddress);
			int j = 0;
			for (; j < buffer[i].numStackEvents; j++) {
				fprintf(fp, "Thread: %llu in Function: %llu\n", stackEvents[j].thread, stackEvents[j].identifier);
			}
			free((struct StackEvent *) stackEvents);
		}

		// reset buffer
		// maybe we can get rid of this memcpy, since we one take bufferElements
//    memset(buffer, 0, WRITE_BUFFER_SIZE * sizeof(struct SampleEvent));
		bufferElements = 0;

		int fErr = fclose(fp);
		fprintf(stdout, "fclose exited with %i\n", fErr);
	} else {
		fprintf(stderr, "Could not open file for writing.\n");
	}
	return NULL;
}

/*
 * We implemented the GNU cyg_profile interface to profile a shadow stack full-instrumentation.
 * Can we determine the thread identifier within this function?
 */
void __cyg_profile_func_enter(void *func, void *callsite) {
#ifdef DEBUG
	fprintf(stderr, "Entering cyg_profile_func_enter \n");
#endif

#ifdef SHADOWSTACK_ONLY
	if(_multithreadStack == 0) {
		fprintf(stderr, "We would call another createStackInstance()\n");
		createStackInstance();
	}
#endif	// SHADOWSTACK_ONLY

	struct StackEvent event;
	event.thread = 0;
	event.identifier = (unsigned long long) func;		// RN: some smaller identifier for performance reasons?
	pushEvent(_multithreadStack[0], event);

#ifdef DEBUG
	fprintf(stderr, "Exit cyg_profile_func_enter \n");
#endif
}

/*
 *
 */
void __cyg_profile_func_exit(void *func, void *callsite) {
#ifdef DEBUG
	fprintf(stderr, "Entering cyg_profile_func_exit \n");
#endif

	popEvent(_multithreadStack[0]);

#ifdef DEBUG
	fprintf(stderr, "Exit cyg_profile_func_exit \n");
#endif
}

/*
 * SAMPLING DRIVER SEGMENT
 */

/* PAPI Sampling handler */
void handler(int EventSet, void *address, long_long overflow_vector, void *context) {
	if (ssReady == 0) {
		return;
	}
	sampleCount++;

	// This is where the work happens
	if (instroUseMultithread > 1) {
		flushStackToBuffer(getStack(PAPI_thread_id()), _flushToDiskBuffer, address);
	} else {
		flushStackToBuffer(getStack(0), _flushToDiskBuffer, address);
	}

}

#ifndef SHADOWSTACK_ONLY
void
#ifndef SAMPLING_AS_LIB
__attribute__ ((constructor))
#endif
init_sampling_driver() {

	/* read environment variable to control the sampling driver */
	char *instroFreqVariable = getenv("INSTRO_SAMPLE_FREQ");
	// get the sample frequency
	if (instroFreqVariable != NULL) {
		printf("Using sample frequency set with INSTRO_SAMPLE_FREQ = %s\n", instroFreqVariable);
		overflowCountForSamples = atoi(instroFreqVariable);
	} else {
		printf("INSTRO_SAMPLE_FREQ not set, using a sample each 2600000 cycles\n");
	}

	printf("With current config we allocate: %li bytes\n",
	WRITE_BUFFER_SIZE * 2 * sizeof(struct SampleEvent));
	initBuffer();
	printf("Enabling PAPI for sampling\n");

	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}

	if (instroUseMultithread > 1) { /* defined in stack.c */
		fprintf(stderr, "Initializing for multithread support.\n");
		if ((retval = PAPI_thread_init(getKey)) != PAPI_OK) {
			errx(retval, "PAPI_thread_init failed with %i", retval);
		}
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
	sampling_driver_enabled = 1;
	printf("Sampling Driver Enabled\n");
}

void
#ifndef SAMPLING_AS_LIB
__attribute__ ((destructor))
#endif
finish_sampling_driver() {
	printf("sampling_tool Finalizer\n");
	if (sampling_driver_enabled) {
		long long instructionCounter;
		PAPI_stop(EventSet, &instructionCounter);
		printf("%li samples taken\n", sampleCount);
		printf("%lli elements in buffer\n", bufferElements);

#ifndef USE_THREAD_WRITE_OUT
		flushBufferToFile(_flushToDiskBuffer);
		pthread_exit(NULL);
#else
//		fprintf(stdout, "Using pthread write out\n");
//		pthread_t writeThread;
//		int err = pthread_attr_setdetachstate(&detachAttr, PTHREAD_CREATE_DETACHED);
//		err = pthread_create(&writeThread, 0, pthread_flushBufferToFile, _flushToDiskBuffer);
#endif

		printf("Sampling Driver Disabled\n");
	} else {
		printf("Sampling was disabled due to an error\n");
	}

#ifdef WITH_MAX_SIZE
	printf("The max stack size reached was: %u\n", stackMaxSize);
#endif

}
#endif	// SHADOWSTACK_ONLY

