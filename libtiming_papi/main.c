#include "timing.h"

void __attribute__ ((noinline)) doWork() {
	double d = 42.0;
	double dd = 3.41;
	double ddd = d + dd;
}


#define MAXITERATIONS 10000000
#define TESTCASE "vanilla"
int main() {
	initMeasurement();
	printResultsHeader();
	{	
		startMeasurement();
		stopMeasurement();
		printResults(0);
	}
	for (int ITERATIONS=10000;ITERATIONS< MAXITERATIONS; ITERATIONS*=10)

	{
		startMeasurement();

		for(int i = 0; i < ITERATIONS; ++i) {
			doWork();
		}

		stopMeasurement();
		printResults(ITERATIONS);
	}
	finalizeMeasurement();
	return 0;
}
