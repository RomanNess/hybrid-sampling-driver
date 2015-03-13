#ifndef H_STACK__
#define H_STACK__
#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "err.h"

#include "pthread.h"

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

/* Represents one single event (function) on the stack */
struct StackEvent {
	unsigned long identifier;
};

/*
 * This datastructure models our stack.
 * _size is the number of elements
 * _maxSize is the maximum number of elements the Stack can hold
 */
struct Stack {

	struct StackEvent *_elements;
	unsigned int _size, _maxSize;

};

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
void pushEvent(struct Stack *stack, struct StackEvent event);
void popEvent(struct Stack *stack);

/*
 * This is the public interface.
 * Calls to these functions should be inserted to target source.
 */
void _instroPushIdentifier(unsigned long long functionIdentifier);
void _instroPopIdentifier();

/* The interface for GNU instrumentation with shadow stack */
void __cyg_profile_func_enter(void *func, void *callsite);
void __cyg_profile_func_exit(void *func, void *callsite);

/* common interface */
inline void pushIdentifier(unsigned long long functionIdentifier) {

	struct StackEvent event;
	event.identifier = (unsigned long long) functionIdentifier;		// RN: some smaller identifier for performance reasons?

	pushEvent(_multithreadStack[threadId], event);
}

inline void popIdentifier() {
	popEvent(_multithreadStack[threadId]);
}

#endif

