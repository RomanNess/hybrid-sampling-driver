

#include "stdio.h"




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


};

void pushEvent(struct Stack* stack, struct StackEvent event);

void popEvent(struct Stack* stack);
