#ifndef H_STACK__
#define H_STACK__

#include <stdio.h>
#include <stdlib.h>
#include <err.h>	// for errx

#include "event.h"

/*
 * TODO I don't know if it makes sense to have a fixed number of stack size
 * This limits our capabilities sampling programs with very very deep call trees.
 * But we only do one allocation at start up
 */
#define STACK_SIZE 100000

/*
 * If the compiler flag is set at compilation time, the shadow stack will tell us which was the maximum
 * stack depth during the sampling run.
 */
#ifdef WITH_MAX_SIZE
unsigned int stackMaxSize;
#endif

/*
 * The number of threads to be used.
 * Filled by env var INSTRO_NUM_THREADS or OMP_NUM_THREADS (in this order)
 */
int instroNumThreads;

/*
 * This is the array of shadow stacks.
 * XXX rename this variable!
 */
extern struct Stack **_multithreadStack;

/*
 * selfmade continuous ids
 */
extern __thread int threadId;
extern volatile int currentThreadNum;
void assingContinuousThreadId();
unsigned long getThreadId();	// for PAPI_init()

void readEnv();
void initShadowStack();


/* Manage a single shadow stack */
void initSingleStack(struct Stack *stack, unsigned int maxSize);
void finiSingleStack(struct Stack *stack);

/*
 * "internal" stack interface
 * pushEvent adds one element (event) to stack (stack)
 * popEvent removes one element from stack (stack)
 */
//void pushEvent(struct Stack *stack, struct StackEvent event);
//void popEvent(struct Stack *stack);
/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
inline void pushEvent(struct Stack *stack, struct StackEvent event) {
#ifdef DEBUG
	fprintf(stderr, "Enter Function: push event:\nstack-base: %p\nstack-size:%i\nadding at stack[size]: %p.\nstack-start: %p\n", stack, stack->_size, &(stack->_elements[stack->_size]), stack->_elements);
#endif

#ifndef MAX_SPEED
	if (stack->_size == stack->_maxSize) {
		fprintf(stderr, "Maximum stack size of %i reached.\n", STACK_SIZE);
	}
#endif

	stack->_elements[stack->_size].identifier = event.identifier;
	stack->_size += 1;

#ifdef DEBUG
	fprintf(stderr, "Leave Function: push event:\nstack-base: %p\nstack-size:%i\nadded element at stack[size]: %p.\nstack-start: %p\n", stack, stack->_size, &(stack->_elements[stack->_size-1]), stack->_elements);
#endif

#ifdef WITH_MAX_SIZE
	if(stack->_size > stackMaxSize) {
		stackMaxSize = stack->_size;
	}
#endif

}

/*
 * Pops an event from the stack.
 * Effectively decreases the size of the stack not freeing any memory.
 * (internal interface)
 */
inline void popEvent(struct Stack *stack) {
	stack->_size -= 1;
}

/*
 * This is the public interface.
 * Calls to these functions should be inserted to target source.
 */
void _instroPushIdentifier(unsigned long functionIdentifier);
void _instroPopIdentifier();

#ifndef NO_CYG_PROF
/* The interface for GNU instrumentation with shadow stack */
void __cyg_profile_func_enter(void *func, void *callsite);
void __cyg_profile_func_exit(void *func, void *callsite);
#endif

/* common interface */
inline void pushIdentifier(unsigned long functionIdentifier) {

	struct StackEvent event;
	event.identifier = functionIdentifier;		// RN: some smaller identifier for performance reasons?

#ifdef SERIAL_OPT
	pushEvent(_multithreadStack[0], event);
#else
	pushEvent(_multithreadStack[threadId], event);
#endif
}

inline void popIdentifier() {
#ifdef SERIAL_OPT
	popEvent(_multithreadStack[0]);
#else
	popEvent(_multithreadStack[threadId]);
#endif
}



#endif

