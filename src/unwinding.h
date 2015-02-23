#ifndef SRC_UNWINDING_H_
#define SRC_UNWINDING_H_

#include <stdio.h>
#include <libunwind.h>
#include <monitor.h>

#define MAX_UNWIND_FACTOR 20	/* max unwind factor */

/**
 * RN 2015-02
 * TODO: unwind only n steps
 * TODO: note that the order of the unwind call stack is the other way around compared to the shadow stack
 */

void doUnwind(int unwindSteps);

#endif /* SRC_UNWINDING_H_ */
