#include <iostream>
#include "timing.h"

//extern "C" {
//	void startMeasurement();
//	void stopMeasurement();
//	void printResults();
//}

// #define ITERATIONS 1

void __attribute__ ((noinline)) doWork() {
	//	std::cout << "executing foo()\n";
	double d = 42.0;
	double dd = 3.41;
	double ddd = d + dd;
}


#define MAXITERATIONS 10000000
#define TESTCASE "vanilla"
int main(int argc, char** argv) {
	fprintf(stderr,"%s",TESTCASE);
	initMeasurement();
	printResultsHeader();
	{	
		startMeasurement();
		stopMeasurement();
		fprintf(stderr,"%s",TESTCASE);
		printResults(0);
	}
	for (int ITERATIONS=10000;ITERATIONS< MAXITERATIONS; ITERATIONS*=10)

	{
		std::cout << "=== start main  ===\n";
		startMeasurement();

		for(int i = 0; i < ITERATIONS; ++i) {
			doWork();
		}

		stopMeasurement();
		std::cout << "=== finish main ===\n";
		fprintf(stderr,"%s",TESTCASE);
		printResults(ITERATIONS);
	}
	finalizeMeasurement();
	return 0;
}
