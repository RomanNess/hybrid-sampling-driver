/*
 * This sample driver implements the InstRO Shadow Stack interface as well as
 * the GCC _cyg_profile_func_enter/exit interface.
 * It uses PAPI to sample at defined cycle rates.
 *
 * Authors:     Jan-Patrick Lehr
 *              Christian Iwainsky
 *              Roman Ness
 */
#ifndef STD_INCLUDES

#define STD_INCLUDES
#include "stdio.h"
#include "stdlib.h"
#include "papi.h"
#include "pthread.h"
#include "string.h" // for memset

#ifndef STACK_IS_UNDER_TEST
#include "math.h"
#endif

#endif	// STD_INCLUDES

/*
 * TODO 2014-05-12 JP: Implement functionality to write the output to a user defined location
 * TODO 2014-05-12 JP: What data is needed per sample event?
 */

#ifndef H_STACK__
#include "stack.h"
#endif
#include "event.h"

// This is our write buffer
static struct SampleEvent *_flushToDiskBuffer = 0;
static unsigned int numberOfBufferElements = 0;

struct Stack **_multithreadStack = 0;

/*
 * GLOBALS SEGMENT
 */
long int sampleCount = 0; /* total overflows */

int sampling_driver_enabled = 0; /* global to know if sampling driver is enabled*/

int EventSet = PAPI_NULL; /* PAPI related thing */

/*
 * CPU-cycles per sample.
 * This can be changes using the env var INSTRO_SAMPLE_FREQ
 */
long overflowCountForSamples = 2600000;

/*
 * This is not a size in bytes or something, but the number of sampling events
 * which will be buffered before they are written to a file.
 */
#define WRITE_BUFFER_SIZE 50000

/*
 * If the compiler flag is set at compilation time, the shadow stack will tell us which was the maximum
 * stack depth during the sampling run.
 */
#ifdef WITH_MAX_SIZE
unsigned int stackMaxSize = 0;
#endif

/*
 * FUNCTIONS 
 */
void initBuffer();

void deallocateWriteOutBuffer();

void flushStackToFile(struct Stack *stack);

void flushStackToBuffer(struct Stack *stack, struct SampleEvent *buffer, void *icAddress);

void flushBufferToFile(struct SampleEvent *buffer);

void __cyg_profile_func_enter(void *func, void *callsite);

void __cyg_profile_func_exit(void *func, void *callsite);

void handler(int EventSet, void *address, long_long overflow_vector, void *context);

void init_sampling_driver();

void finish_sampling_driver();

/*
 * This is needed for the new asynchronous write-out functionality
 */
pthread_t writeThread;
pthread_attr_t detachAttr;
void* pthread_flushBufferToFile(void *data);
