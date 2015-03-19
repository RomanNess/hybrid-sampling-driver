/*
 * This sample driver implements the InstRO Shadow Stack interface as well as
 * the GCC _cyg_profile_func_enter/exit interface.
 * It uses PAPI to sample at defined cycle rates.
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

#ifdef USE_CPP_LIB
#include "cpp/hash.h"
#endif

#ifndef SHADOWSTACK_ONLY
#include "unwinding.h"
#endif

/*
 * TODO 2014-05-12 JP: Implement functionality to write the output to a user defined location
 * TODO 2014-05-12 JP: What data is needed per sample event?
 * TODO 2015-02 RN: move libmonitor stuff to other file
 * TODO 2015-02 RN: Consider that the current call stack is missing in new threads (e.g. in parallel regions)
 */

#include "stack.h"
#include "event.h"

/* the actual shadow stack (for multithreading) */
struct Stack **_multithreadStack = 0;

/* write buffer */
static struct SampleEvent *_flushToDiskBuffer = 0;
static unsigned int numberOfBufferElements = 0;

long int sampleCount = 0; /* total samples taken */
long overflowCountForSamples = 2600000; /* CPU-cycles per sample (set by INSTRO_SAMPLE_FREQ) */

#define WRITE_BUFFER_SIZE 50000	/* number of sampling events in the buffer */
__thread int EventSet = PAPI_NULL; /* PAPI related thing */


/*
 * FUNCTIONS 
 */
void initBuffer();
void finiBuffer();

void flushStackToFile(struct Stack *stack);
void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer, void *icAddress);
void flushBufferToFile(struct SampleEvent *buffer);

void handler(int EventSet, void *address, long long overflow_vector, void *context);

void initSamplingDriver();
void registerThreadForPAPI();
void finishSamplingDriver();

#endif	// DRIVER_H
