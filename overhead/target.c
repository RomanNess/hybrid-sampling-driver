
#ifdef META_BENCHMARK
#include "libtiming_papi/timing.h"
#endif

#define TARGET_ITERATIONS 1000*10000


void rec10() {}

void rec9() {
	rec10();
}

void rec8() {
	rec9();
}

void rec7() {
	rec8();
}

void rec6() {
	rec7();
}

void rec5() {
	rec6();
}

void rec4() {
	rec5();
}

void rec3() {
	rec4();
}

void rec2() {
	rec3();
}

void rec1() {
	rec2();
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
