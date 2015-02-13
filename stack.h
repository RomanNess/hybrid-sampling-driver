#ifndef H_STACK__
#define H_STACK__
#include "stdio.h"
#include "stdlib.h"
#include "limits.h"

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
 * At the moment this key is used to determine a threads id. Therefore
 * it needs to be declared as extern __thread in the target application.
 * This would be one possibility, but we would need to have a mapping
 * between the pthread IDs and the instro id, if we do it that way.
 * XXX JP: This seems to be the faster way!
 */
extern __thread pthread_key_t key;
unsigned long getKey();

/*
 * Starting here this is another idea: We keep track of threads ourselves
 * and use a lookup function that we provide. That way we would use thread
 * local storage to have a thread id inside each thread. We would provide
 * papi with our own get_thread_id function.
 * We really have to evaluate in terms of performance and maintainability.
 *
 */
volatile unsigned int maxThreadNr;
extern __thread unsigned long _instro_thread_id;
long unsigned int instro_get_thread_id();
/***/

/*
 * "internal" stack interface
 * pushEvent adds one element (event) to stack (stack)
 * popEvent removes one element from stack (stack)
 */
void pushEvent(struct Stack *stack, struct StackEvent event);
void popEvent(struct Stack *stack);

/*
 * Initializes the stack.
 * The function calls malloc
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
 * At the moment the events are create in the _instroPushIdentifier function.
 * The correct stack corresponding to threadIdentifier is selected and then pushEvent
 * is called with these two things as parameters.
 */
void _instroPushIdentifier(unsigned long long functionIdentifier, unsigned long long threadIdentifier);
/*
 * Calls popEvent with the correct stack as argument.
 */
void _instroPopIdentifier(unsigned long long threadIdentifier);

/*
 * Used in the PAPI handler function, each time the sample is taken.
 * This has to do with how we handle threads. I believe there is some
 * better solution to all this, but I havent had the idea so far.
 */
struct Stack *getStack(unsigned long threadIdentifier);

#endif

