#include "stack.h"

int instroNumThreads;

__thread pthread_key_t threadId = NO_THREAD_ID;
volatile unsigned int currentThreadNum = 0;

#ifdef WITH_MAX_SIZE
unsigned int stackMaxSize = 0;
#endif

unsigned long getThreadId() {
	return threadId;
}

void assingContinuousThreadId() {

	if (currentThreadNum >= instroNumThreads) {
		errx(1, "Assigned more than %u threads.", instroNumThreads);
	}
	threadId = currentThreadNum++;

	///XXX
	printf("# created key: %u \n", threadId);
}

void readEnv() {
	char *instroNumThreadsEnvVariable = getenv("INSTRO_NUM_THREADS");
	char *ompNumThreadsEnvVariable = getenv("OMP_NUM_THREADS");

	// get the number of threads
	if (instroNumThreadsEnvVariable != NULL) {
		instroNumThreads = atoi(instroNumThreadsEnvVariable);
	} else if (ompNumThreadsEnvVariable != NULL) {
		instroNumThreads = atoi(ompNumThreadsEnvVariable);
	} else {
		instroNumThreads = 1;
	}
	printf("Running with %i threads.\n", instroNumThreads);
}

void initShadowStack() {

	// XXX Is this check necessary?
	if (_multithreadStack == NULL) {
#ifdef DEBUG
		fprintf(stderr, "Allocating stack **\n");
#endif
		_multithreadStack = (struct Stack **) malloc(instroNumThreads * sizeof(struct Stack *));
		if (_multithreadStack == 0) {
			fprintf(stderr, "Allocation failed in %s in %s\n", __FILE__, __FUNCTION__);
		}
#ifdef DEBUG
		if(_multithreadStack[0] == 0) {
			fprintf(stderr, "if(_multithreadStack[0] == 0) -- true\n");
		} else {
			fprintf(stderr, "ThreadStack: %p \n", _multithreadStack);
		}
#endif
		for (int i = 0; i < instroNumThreads; i++) {
			_multithreadStack[i] = (struct Stack *) malloc(sizeof(struct Stack));
			if (_multithreadStack[i] == NULL) {
				fprintf(stderr, "Could not allocate memory for multithread stack\n");
			}

			initSingleStack(_multithreadStack[i], STACK_SIZE);
		}
#ifdef DEBUG
		if(_multithreadStack[0] == 0) {
			fprintf(stderr, "IF YOU CAN READ THIS TEXT, IT IS BAD\n");
		}
#endif

		fprintf(stderr, "Ready flag set\n");
	}
#ifdef DEBUG
	fprintf(stderr, "end of createStackInstance()\n");
#endif
}

void initSingleStack(struct Stack *stack, unsigned int maxSize) {

	// initialize stack
	stack->_elements = (struct StackEvent *) malloc(maxSize * sizeof(struct StackEvent));
	if (stack->_elements == NULL) {
		fprintf(stderr, "Could not allocate shadowstack with max size: %u\n", maxSize);
		exit(-1);
	}
	stack->_maxSize = maxSize;
	stack->_size = 0;
#ifdef DEBUG
	fprintf(stderr, "Init Stack:\nStack base: %p\nstack->start at: %p\n", stack, stack->_elements);
#endif
}

/*
 * Deallocates the stack and frees the memory
 */
void finiSingleStack(struct Stack *stack) {
	free(stack->_elements);
}

/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
void pushEvent(struct Stack *stack, struct StackEvent event) {
#ifdef DEBUG
	fprintf(stderr, "Enter Function: push event:\nstack-base: %p\nstack-size:%i\nadding at stack[size]: %p.\nstack-start: %p\n", stack, stack->_size, &(stack->_elements[stack->_size]), stack->_elements);
#endif

	if (stack->_size == stack->_maxSize) {
		fprintf(stderr, "Maximum stack size of %i reached.\n", STACK_SIZE);
	}

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
void popEvent(struct Stack *stack) {
	stack->_size -= 1;
}

/*
 * Pushes an identifier to the stack.
 * (Public Interface)
 */
void _instroPushIdentifier(unsigned long long functionIdentifier) {
	pushIdentifier(functionIdentifier);
}

/*
 * Removes a stack event from the threadIdentifier corresponding stack.
 * (Public Interface)
 */
void _instroPopIdentifier() {
	popIdentifier();
}

void __cyg_profile_func_enter(void *func, void *callsite) {
#ifdef DEBUG
	fprintf(stderr, "Entering cyg_profile_func_enter \n");
#endif

	pushIdentifier( (unsigned long long) func);

#ifdef DEBUG
	fprintf(stderr, "Exit cyg_profile_func_enter \n");
#endif
}

void __cyg_profile_func_exit(void *func, void *callsite) {
#ifdef DEBUG
	fprintf(stderr, "Entering cyg_profile_func_exit \n");
#endif

	popIdentifier();

#ifdef DEBUG
	fprintf(stderr, "Exit cyg_profile_func_exit \n");
#endif
}


