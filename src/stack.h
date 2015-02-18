#ifndef H_STACK__
#define H_STACK__
#include "stdio.h"
#include "stdlib.h"
#include "limits.h"

#include "pthread.h"

// TODO RN 2015-02: add unique push/pop mechanism for cyg_profile & _instro interfaces
// TODO RN 2015-02: Consider that the current call stack is missing in newly forked threads

#define NO_THREAD_ID -1

/*
 * TODO I don't know if it makes sense to have a fixed number of stack size
 * This limits our capabilities sampling programs with very very deep call trees.
 * But we only do one allocation at start up
 */
#define STACK_SIZE 100000

/*
 * Represents one single event on the stack (a certain thread is in function with identifier)
 * XXX JP: Since we have multiple stacks, may be we can omit the thread identifier per
 * StackEvent, so we would further decrease memory footprint.
 *
 */
struct StackEvent {

	unsigned long long thread;
	unsigned long long identifier;

};

/*
 * This datastructure models our stack.
 * XXX _cur is not really in use at the moment. I dont know if it is any useful..
 * _size is the number of elements
 * _maxSize is the maximum number of elements the Stack can hold
 * _initialized was used to indicate between driver and stack whether the stack was initialized
 *              or not. XXX Maybe this can be removed, too.
 */
struct Stack {

	struct StackEvent *_start, _end, _cur;
	unsigned int _size, _maxSize;
	short _initialized;

};

/*
 * The number of threads to be used.
 * Filled by env var INSTRO_NUM_THREADS or OMP_NUM_THREADS (in this order)
 */
int instroNumThreads;
/*
 * This is the array of shadow stacks.
 *
 * XXX rename this variable!
 */
extern struct Stack **_multithreadStack;

/*
 * We need some flag to indicate, whether we can start to sample events.
 * If that is a good way.. I doubt it, especially since it seems that the
 * shadow stack constructor is run _before_ the sampling initializer.
 * XXX I think if we really need this we should think about atomic_bool
 * or similar things.
 */
volatile int ssReady;

/*
 * selfmade continuous ids
 */
extern __thread pthread_key_t key;
extern volatile unsigned int counter;
void assingContinuousThreadId();
unsigned long getThreadId();


/*
 * "internal" stack interface
 * pushEvent adds one element (event) to stack (stack)
 * popEvent removes one element from stack (stack)
 */
void pushEvent(struct Stack *stack, struct StackEvent event);
void popEvent(struct Stack *stack);

/*
 * Initializes the stack (malloc)
 */
void initStack(struct Stack *stack, unsigned int maxSize);
/*
 * Deallocates the stack.
 * XXX This function needs some attention!
 * This function actually calls free.
 */
void deallocateStack(struct Stack *stack);

/*
 * This is the public interface.
 * Calls to these functions should be inserted to target source.
 */
void _instroPushIdentifier(unsigned long long functionIdentifier);
void _instroPopIdentifier();

/*
 * The interface for GNU instrumentation with shadow stack
 */
void __cyg_profile_func_enter(void *func, void *callsite);
void __cyg_profile_func_exit(void *func, void *callsite);


#endif

