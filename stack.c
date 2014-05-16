
#include "stack.h"

__thread pthread_key_t key = 0;
__thread long unsigned int _instro_thread_id = 0;
volatile int ssReady = 0;
volatile unsigned int maxThreadNr = 1;

long unsigned int instro_get_thread_id(){
	if(_instro_thread_id == 0){
		_instro_thread_id = maxThreadNr;
		maxThreadNr++;
//		fprintf(stderr, "instro_thread_identifier set\n");
	}

	return _instro_thread_id;
}

pthread_key_t getKey(){
	if(key == 0)
		pthread_key_create(&key, 0);

	return key;
}

void
__attribute__((constructor))
createStackInstance(){
	// XXX JP: I added the getting of the multi thread env var here, because we have to know at creation
	// time whether its multi thread or not...
	char *instroUseMultithreadVariable = getenv("INSTRO_USE_THREAD_NUMBER");
	char *ompNumThreadsEnvVariable = getenv("OMP_NUM_THREADS");

	// get the number of threads
	if(instroUseMultithreadVariable != NULL){
		instroUseMultithread = atoi(instroUseMultithreadVariable);
	} else if(ompNumThreadsEnvVariable != NULL){
		instroUseMultithread = atoi(ompNumThreadsEnvVariable);
	} else {
		printf("Not using multithreading, setting thread level to one thread\n");
		instroUseMultithread = 1;
	}


	if(_multithreadStack == 0){
		_multithreadStack = (struct Stack**) malloc(instroUseMultithread * sizeof(struct Stack*));
		int i = 0;
		for(; i < instroUseMultithread; i++){
			_multithreadStack[i] = (struct Stack*) malloc(sizeof(struct Stack));
			if(! _multithreadStack[i])
				fprintf(stderr, "Could no allocate memory for multithread stack\n");

//			fprintf(stderr, "createStackInstance:\nstack-base  %i: %p\nNow initializing...",i, _multithreadStack[i]);
			initStack(_multithreadStack[i], STACK_SIZE);
		}

	/* What happens if that is moved here... */
	if(key == 0){
		pthread_key_create(&key, 0);
		printf("Create Stack Instance:\nIn Shadow stack creating key for thread: %u with key: %u\n", pthread_self(), key);
	}

//	fprintf(stderr, "Construction done for %i threads.\n", i);
	ssReady = 1;
	fprintf(stderr, "Ready flag set\n");
	}
}

void initStack(struct Stack* stack, unsigned int maxSize){
//if(stack != NULL)
//fprintf(stderr, "initStack neq NULL: %p\n", stack);
	if(stack->_initialized == 1)
		return;

	// initialize stack
	stack->_start = (struct StackEvent*) malloc(maxSize * sizeof(struct StackEvent));
	if(stack->_start == NULL){
		fprintf(stderr, "Could not allocate shadowstack with max size: %u\n", maxSize);
		exit(-1);
	}
	stack->_maxSize = maxSize;
	stack->_end = stack->_start[maxSize-1];
	stack->_cur = stack->_start[0];
	stack->_size = 0;
	stack->_initialized = 1;

	fprintf(stderr, "Init Stack:\nStack base: %p\nstack->start at: %p\n", stack, stack->_start);
}


/*
 * Pushes a stack event to the stack
 * (internal interface)
 */
void pushEvent(struct Stack* stack, struct StackEvent event){
//	fprintf(stderr, "Function Enter: push event:\nstack-base: %p\nstack-size:%i\nadding element at stack[size]: %p.\nstack base: %p\n", stack, stack->_size, &(stack->_start[stack->_size]), stack->_start);
//	fflush(stderr);
	if(stack->_size == stack->_maxSize){
		fprintf(stderr, "Maximum stack size of %i reached.\n", STACK_SIZE);
	}

	stack->_start[stack->_size].thread = event.thread;
	stack->_start[stack->_size].identifier = event.identifier;
	stack->_size += 1;
//	fprintf(stderr, "Function Leave: push event:\nstack-base: %p\nstack-size:%i\nadded element at stack[size]: %p.\nstack base: %p\n", stack, stack->_size, &(stack->_start[stack->_size]), stack->_start);

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



/*
 * Pushes an identifier to the stack corresponding to threadIdentifier.
 * This is our public interface for the InstRO Sampling driver.
 */
void _instroPushIdentifier(unsigned long long functionIdentifier, unsigned long long threadIdentifier){
/*
 * XXX JP: I guess if we use the way with our own lookup function for thread identifier,
 * 	we do not need the call to the pthread key create thing... 
 */
	if(key == 0){
		pthread_key_create(&key, 0);
		threadIdentifier = key;
		printf("In Shadow stack creating key for thread: %u with key: %u\n", pthread_self(), key);
	}

//	fprintf(stderr, "Pushing %i to thread %i and key is: %lu\n", functionIdentifier, threadIdentifier, key);
//	fprintf(stderr, "Pushing %i to thread %i\n", functionIdentifier, threadIdentifier);
//	fflush(stderr);
	struct StackEvent event;
	event.thread = threadIdentifier;
	event.identifier = functionIdentifier;

//	fprintf(stderr, "Push Identifier:\nThreadidentifier: %lu\nstack-base: %p\n", threadIdentifier, _multithreadStack[threadIdentifier]);
	if(threadIdentifier > instroUseMultithread){
		fprintf(stderr, "ERROR: Requestin stack for thread ID > %i\n", instroUseMultithread);
	}
	
	struct Stack* st = _multithreadStack[threadIdentifier-1];
	pushEvent(st, event);
}


/*
 * Removes a stack event from the threadIdentifier corresponding stack.
 * This is our public interface for the InstRO sampling.
 */
void _instroPopIdentifier(unsigned long long threadIdentifier){
	popEvent(_multithreadStack[threadIdentifier-1]);
}


struct Stack* getStack(unsigned long long threadIdentifier){
	if(threadIdentifier > instroUseMultithread){
		fprintf(stderr, "Requested a stack to a thread with a greater thread number than specified. %llu\n", threadIdentifier);
	}
//	fprintf(stderr, "Using key-1 : %u to fetch the shadow stack.\n", key-1);
	return _multithreadStack[key-1];
}
