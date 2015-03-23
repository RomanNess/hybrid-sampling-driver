#ifndef SRC_UNWINDING_H_
#define SRC_UNWINDING_H_

#include <stdio.h>
#include <stdlib.h> // malloc
#include <libunwind.h>
#include <monitor.h>

#include "cpp/hash.h"
#include "event.h"

#define MAX_UNWIND_FACTOR 20	/* max unwind factor */

unsigned long regionStart, regionEnd;

long doUnwind(unsigned long address, void* context, struct SampleEvent *buffer);

#endif /* SRC_UNWINDING_H_ */
