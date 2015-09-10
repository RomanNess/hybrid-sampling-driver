#ifdef META_BENCHMARK
#include "libtiming_papi/timing.h"
#endif

#define TARGET_ITERATIONS 1000*1000*100

void rec1() {
}

int main() {

#ifdef META_BENCHMARK
	stopMeasurement();
	printResults("init");
	startMeasurement();
#endif

	for (int i = 0; i < TARGET_ITERATIONS; i++) {
		rec1();
	}

#ifdef META_BENCHMARK
	stopMeasurement();
	printResults("main");
	startMeasurement();
#endif

	return 0;
}
