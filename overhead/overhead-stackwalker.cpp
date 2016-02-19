
#include "libtiming/timing.h"

#include <walker.h>
#include <frame.h>
#include <vector>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

//#define NO_UNWIND
#define UNW_INIT_ONLY
#define PRINT_FUNCTIONS 0

using namespace Dyninst::Stackwalker;

int calldepth = -1;

/* determine the overhead of stackwalker api */

inline void printFrame(const Frame& frame) {
	std::string s;
	frame.getName(s);

	std::cout << "\t" << s << " - RA: " << std::hex << frame.getRA()
			<< " - isBottom:" << frame.isBottomFrame() << std::endl;
}

extern "C" {
void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	calldepth++;

	startMeasurement();

#ifndef NO_UNWIND

	Walker* walker = Walker::newWalker();
//	Frame frame(walker);
//	walker->getInitialFrame(frame);

#if  PRINT_FUNCTIONS
	printFrame(frame);
#endif


#ifndef UNW_INIT_ONLY

	int unwindSteps = calldepth;

	int status = 1;
	while (unwindSteps != 0) {

		unwindSteps--;
		Frame tmpFrame(walker);
		bool result = walker->walkSingleFrame(frame, tmpFrame);
		if (!result) {
			std::cout << "Stack walking failed." << std::endl;
			break;
		}
#if  PRINT_FUNCTIONS
		printFrame(tmpFrame);
#endif

		frame = tmpFrame;
	}

#endif	// UNW_INIT_ONLY

#endif	// NO_UNWIND

	stopMeasurement();

	char depth[65];
	sprintf(depth, "%d", calldepth);
	printResults(depth);
}

void __cyg_profile_func_exit(void* func_ptr, void* call_site) {
	calldepth--;
	return;
}

void *monitor_init_process(int* argc, char** argv, void* data) {

	initMeasurement();
	printResultsHeader();

	startMeasurement();
	stopMeasurement();
	printResults("warmup");

	for(int i=0; i<5; i++) {
		startMeasurement();
		stopMeasurement();
		printResults("ref");
	}

	return NULL;
}

void monitor_fini_process(int how, void* data) {
	finalizeMeasurement();
}


}	// extern "C"

int main() {

	return 0;
}
