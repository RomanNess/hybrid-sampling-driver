
#include "stack.h"

/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
void pushEvent(struct Stack* stack, struct StackEvent event){
#ifdef SHADOWSTACK_ONLY
	if(stack->_initialized != 1)
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


void initStack(struct Stack* stack, unsigned int maxSize){
fprintf(stderr, "initStack\n");
	if(stack->_initialized == 1)
		return;

	// initialize stack
	stack->_start = (struct StackEvent*) malloc(maxSize * sizeof(struct StackEvent));
	if(stack->_start == 0){
		fprintf(stderr, "Could not allocate shadowstack with max size: %u\n", maxSize);
		exit(-1);
	}
	stack->_maxSize = maxSize;
	stack->_end = stack->_start[maxSize-1];
	stack->_cur = stack->_start[0];
	stack->_size = 0;
	stack->_initialized = 1;

//	printf("Stack initialized with max size of %u elements\n", stack->_maxSize);
#ifdef SHADOWSTACK_ONLY
	initBuffer();
#endif
}

/*
 * Deallocates the stack and frees the memory
 */
void deallocateStack(struct Stack* stack){
#ifdef STACK_IS_UNDER_TEST	
	flushStackToFile(stack);
#endif
	free(stack->_start);
	stack->_initialized = 0;
}
