
/*
 * This sample driver implements the InstRO Shadow Stack interface as well as
 * the GCC _cyg_profile_func_enter/exit interface.
 * It uses PAPI to sample at defined cycle rates.
 * 
 * Authors:	Jan-Patrick Lehr
 * 		Christian Iwainsky
 */
#ifndef STD_INCLUDES
#define STD_INCLUDES
#include "stdio.h"
#include "stdlib.h"
#include "papi.h"
#include "pthread.h"
#ifndef STACK_IS_UNDER_TEST
	#include "math.h"
#endif
#include "string.h" // for memset
#endif

/*
 * TODO 2014-05-12 JP: Implement functionality to write the output to a user defined location
 * TODO 2014-05-12 JP: What data is needed per sample event?
 */

#ifndef H_STACK__
#include "stack.h"
#endif
#include "event.h"


/*
 * This is the global for a thread's stack. At the moment we are only supporting single thread.
 * XXX DEPRECATED
 */
static struct Stack* _threadStack = 0;

// This is our write buffer
static struct SampleEvent* _flushToDiskBuffer = 0;
/*
 * NUMBER of elements in buffer.
 * TODO Rename this variable
 */
static unsigned int bufferElements = 0;


/*
 * GLOBALS SEGMENT
 */
long int sampleCount = 0;      /* total overflows */

int sampling_driver_enabled=0; /* global to know if sampling driver is enabled*/

int EventSet = PAPI_NULL; /* PAPI related thing */


/*
 * We use a standard of a sample each 2,600,00 cpu cycles 
 * This can be changes using the env var INSTRO_SAMPLE_FREQ
 */
long overflowCountForSamples=2600000; // Now we sample every 2 600 000 cycles


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



