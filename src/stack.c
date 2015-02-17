#include "stack.h"

__thread pthread_key_t key = 0;
__thread unsigned long _instro_thread_id = 0;
volatile int ssReady = 0;
volatile unsigned int maxThreadNr = 1;

unsigned long instro_get_thread_id() {
	if (_instro_thread_id == 0) {
		_instro_thread_id = maxThreadNr;
		maxThreadNr++;
	}
	return _instro_thread_id;
}

unsigned long getKey() {
	if (key == 0) {
		pthread_key_create(&key, 0);
	}
	return (unsigned long) key;
}

void
__attribute__((constructor))
createStackInstance() {

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
		int i = 0;
		for (; i < instroNumThreads; i++) {
			_multithreadStack[i] = (struct Stack *) malloc(sizeof(struct Stack));
			if (_multithreadStack[i] == NULL) {
				fprintf(stderr, "Could not allocate memory for multithread stack\n");
			}

			initStack(_multithreadStack[i], STACK_SIZE);
		}
#ifdef DEBUG
		if(_multithreadStack[0] == 0) {
			fprintf(stderr, "IF YOU CAN READ THIS TEXT, IT IS BAD\n");
		}
#endif
		/* What happens if that is moved here... */
		if (key == 0) {
			pthread_key_create(&key, 0);
			printf("Create Stack Instance:\nIn Shadow stack creating key for thread: %lu with key: %u\n",
					pthread_self(), key);
		}

		ssReady = 1;
		fprintf(stderr, "Ready flag set\n");
	}
#ifdef DEBUG
	fprintf(stderr, "end of createStackInstance()\n");
#endif
}

void initStack(struct Stack *stack, unsigned int maxSize) {
	if (stack->_initialized == 1) {
		return;
	}

	// initialize stack
	stack->_start = (struct StackEvent *) malloc(maxSize * sizeof(struct StackEvent));
	if (stack->_start == NULL) {
		fprintf(stderr, "Could not allocate shadowstack with max size: %u\n", maxSize);
		exit(-1);
	}
	stack->_maxSize = maxSize;
	stack->_end = stack->_start[maxSize - 1];
	stack->_cur = stack->_start[0]; // XXX I think we can delete the _cur field.
	stack->_size = 0;
	stack->_initialized = 1;
#ifdef DEBUG
	fprintf(stderr, "Init Stack:\nStack base: %p\nstack->start at: %p\n", stack, stack->_start);
#endif
}

/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
void pushEvent(struct Stack *stack, struct StackEvent event) {
#ifdef DEBUG
	fprintf(stderr, "Enter Function: push event:\nstack-base: %p\nstack-size:%i\nadding at stack[size]: %p.\nstack-start: %p\n", stack, stack->_size, &(stack->_start[stack->_size]), stack->_start);
#endif

	if (stack->_size == stack->_maxSize) {
		fprintf(stderr, "Maximum stack size of %i reached.\n", STACK_SIZE);
	}

	stack->_start[stack->_size].thread = event.thread;
	stack->_start[stack->_size].identifier = event.identifier;
	stack->_size += 1;

#ifdef DEBUG
	fprintf(stderr, "Leave Function: push event:\nstack-base: %p\nstack-size:%i\nadded element at stack[size]: %p.\nstack-start: %p\n", stack, stack->_size, &(stack->_start[stack->_size-1]), stack->_start);
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
 * Deallocates the stack and frees the memory
 */
void deallocateStack(struct Stack *stack) {
#ifdef STACK_IS_UNDER_TEST
	flushStackToFile(stack);
#endif
	free(stack->_start);
	stack->_initialized = 0;
}

/*
 * Pushes an identifier to the stack.
 * This is our public interface for the InstRO Sampling driver.
 */
void _instroPushIdentifier(unsigned long long functionIdentifier) {

	if (key == 0) {
		pthread_key_create(&key, 0);
		printf("In Shadow stack creating key for thread: %lu with key: %u\n", pthread_self(), key);
	}

	struct StackEvent event;
	event.thread = key;
	event.identifier = functionIdentifier;

#ifdef DEBUG
	fprintf(stderr, "Push Identifier:\nThreadidentifier: %lu\nstack-base: %p\n",
			threadIdentifier, _multithreadStack[threadIdentifier]);
#endif
	if (key > instroNumThreads) {
		fprintf(stderr, "ERROR: Requesting stack for thread ID > %i\n", instroNumThreads);
		abort();
	}
#ifdef DEBUG
	fprintf(stderr, "Retrieving stack for threadIdentifier %i\n", threadIdentifier -1);
#endif
	struct Stack *st = _multithreadStack[key - 1];
	pushEvent(st, event);
}

/*
 * Removes a stack event from the threadIdentifier corresponding stack.
 * This is our public interface for the InstRO sampling.
 */
void _instroPopIdentifier() {
	popEvent(_multithreadStack[key - 1]);
}

struct Stack *getStack(unsigned long threadIdentifier) {
	if (threadIdentifier < 0 || threadIdentifier > instroNumThreads) {
		fprintf(stderr, "Requested the stack for an invalid threadIdentifier: %lu\n", threadIdentifier);
		abort();
	}
	return _multithreadStack[threadIdentifier];
}

