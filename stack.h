

#include "stdio.h"
#include "stdlib.h"



/*
 * TODO I don't know if it makes sense to have a fixed number of stack size
 * This limits our capabilities sampling programs with very very deep call trees.
 */
#define STACK_SIZE 100000

/*
 * Represents one single event on the stack (a certain thread is in function with identifier)
 */
struct StackEvent {

	unsigned long long thread;
	unsigned long long identifier;

};

// A stack for our StackEvents
struct Stack {

	struct StackEvent* _start, _end, _cur;
	unsigned int _size, _maxSize;
	short _initialized;

};

/*
 * This is an array of threads we need to track during application runtime.
 * XXX 2014-05-12 JP: How do we know the number of threads we need to capture?
 * XXX JP: Is this a possible way to go...?
 */
//#define NUMBER_OF_THREADS 4
int instroUseMultithread;
static struct Stack** _multithreadStack = 0;



void pushEvent(struct Stack* stack, struct StackEvent event);
void popEvent(struct Stack* stack);

void initStack(struct Stack* stack, unsigned int maxSize);
void deallocateStack(struct Stack* stack);

void _instroPushIdentifier(unsigned long long functionIdentifier, unsigned long long threadIdentifier);
void _instroPopIdentifier(unsigned long long threadIdentifier);

struct Stack* getStack(unsigned long long threadIdentifier);
