
#include "stack.h"

/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
void pushEvent(struct Stack* stack, struct StackEvent event){
#ifdef SHADOWSTACK_ONLY
	if(_initialized == 0)
		initStack(stack, STACK_SIZE);
#endif
	
	if(stack->_size == stack->_maxSize){
		fprintf(stderr, "Maximum stack size of %i reached.\n", STACK_SIZE);
	}

	stack->_start[stack->_size] = event;
	stack->_size++;
#ifdef WITH_MAX_SIZE
	if(stack->_size > stackMaxSize)
		stackMaxSize = stack->_size;
#endif

}

/*
 * Pops an event from the stack.
 * Effectively decreases the size of the stack not freeing any memory.
 * (internal interface)
 */
void popEvent(struct Stack* stack){
/*
 * If this if/else is in use the pop operation is extremely expensive
 * Generally the pop operation seems to be more expensive than the push operation...
 */
	stack->_size--;
}


