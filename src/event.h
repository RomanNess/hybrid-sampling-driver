/*
 * To speed up, we buffer stack events to memory and hope, that we only need to write once!
 * A sample event at the moment includes the following:
 *      thread			- a continuous thread identifier
 *      sampleNumber    - a continuous number identifying the sample
 *      icAddress       - the PAPI instruction counter Address where the sample occured
 *      stackEvents     - this is a copy of our shadow stack at the point in time where the sample occured
 *      numStackEvents  - how many stack elements have been copied.
 *
 * TODO JP: I think that this basic interface should be changed at some point in time.
 */
struct SampleEvent {

	unsigned int thread;
	long int sampleNumber;
	long int icAddress;
	// this is the shadow stack at the point the sample driver interrupted
	struct StackEvent *stackEvents;
	int numStackEvents;

};

