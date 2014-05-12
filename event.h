
/*
 * To speed up, we buffer stack events to memory and hope, that we only need to write once!
 * A sample event at the moment includes the following:
 * 	sampleNumber 	- a simple number identifying the sample
 * 	icAddress	- the PAPI instruction counter Address where the sample occured
 * 	stackEvents	- this is a copy of our shadow stack at the point in time where the sample occured
 *	numStackEvents	- how many stack elements have been copied.
 *
 * TODO I think that this basic interface should be changed at some point in time.
 */
struct SampleEvent {
	
	long int sampleNumber;
	long int icAddress;
	// this is the shadow stack at the point the sample driver interrupted
	struct StackEvent* stackEvents;
	int numStackEvents;

};


