/*
 * This sample driver implements the InstRO Shadow Stack interface as well as
 * the GCC _cyg_profile_func_enter/exit interface.
 *
 * Authors:     Jan-Patrick Lehr
 *              Christian Iwainsky
 *              Roman Ness
 */
#ifndef DRIVER_H
#define DRIVER_H

#include <papi.h>
#include <err.h>	// for errx
#include <unistd.h>	// for getpid

#include "monitor.h"

#ifndef NO_CPP_LIB
#include "cpp/hash.h"
#endif

#include "unwinding.h"

/*
 * TODO 2014-05-12 JP: Implement functionality to write the output to a user defined location
 * TODO 2014-05-12 JP: What data is needed per sample event?
 * TODO 2015-02 RN: move libmonitor stuff to other file
 * TODO 2015-02 RN: Consider that the current call stack is missing in new threads (e.g. in parallel regions)
 */

#include "stack.h"
#include "event.h"

/* the actual shadow stack (for multithreading) */
extern struct Stack **_multithreadStack;

/* write buffer */
#define WRITE_BUFFER_SIZE 1000000	/* number of sampling events in the buffer */
extern struct SampleEvent *_flushToDiskBuffer;
extern unsigned int numberOfBufferElements;

extern long samplesTaken; /* total samples taken */
extern long samplesInDriverRegion; /* samples taken in regions of this driver */
extern long overflowCountForSamples; /* CPU-cycles per sample (set by INSTRO_SAMPLE_FREQ) */

#ifndef NO_PAPI_DRIVER
extern __thread int EventSet; /* PAPI related thing */
#endif

#ifdef ITIMER_DRIVER
static struct itimerval itimer;
static int itimerLock;
static long samplesOmitted;
#endif

/*
 * FUNCTIONS 
 */
void initBuffer();
void finiBuffer();

void flushStackToFile(struct Stack *stack);
void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer);
void flushBufferToFile(struct SampleEvent *buffer);

void handler(int EventSet, void *address, long long overflow_vector, void *context);
inline
void abstractHandler(unsigned long address, void* context) {
	// This is where the work happens
	flushStackToBuffer(_multithreadStack[threadId], _flushToDiskBuffer);

	long startAddress;
	if (driverRegionStart < address && address < driverRegionEnd) {
		startAddress = 0;	// sample in driver region (call context already known)
		samplesInDriverRegion++;
	} else {
		startAddress = doUnwind(address, context, &_flushToDiskBuffer[numberOfBufferElements]);
	}
	_flushToDiskBuffer[numberOfBufferElements].icAddress = startAddress;

	numberOfBufferElements++;
}

void initSamplingDriver();
void initPapiSamplingDriver();
void initItimerSamplingDriver();
void registerThreadForPAPI();
void finishSamplingDriver();

#endif	// DRIVER_H
