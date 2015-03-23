#ifndef SRC_EVENT_H
#define SRC_EVENT_H

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
 * To speed up, we buffer stack events to memory and hope, that we only need to write once!
 * A sample event at the moment includes the following:
 *      thread			- a continuous thread identifier
 *      sampleNumber    - a continuous number identifying the sample
 *      icAddress       - the identifier (address) of the function where the sample was triggered
 *      stackEvents     - this is a copy of our shadow stack at the point in time where the sample occured
 *      numStackEvents  - how many stack elements have been copied.
 *
 *      unwindEvents	- the unwound functions
 *      numUnwindEvents	- the number of unwound functions
 *
 * TODO this basic interface should be changed at some point in time.
 */
struct SampleEvent {

	int thread;
	long int sampleNumber;
	long int icAddress;
	// this is the shadow stack at the point the sample driver interrupted
	struct StackEvent* stackEvents;
	int numStackEvents;

	// XXX unwind stack frames saved separately for convenience
	struct StackEvent* unwindEvents;
	int numUnwindEvents;

};

#endif // SRC_EVENT_H

