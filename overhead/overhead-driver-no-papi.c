
#include "../libtiming_papi/timing.h"

#include "../src/driver.h"

int main() {

	initBuffer();	// allocate buffer

	/* init libsampling */
	parseFunctions("nm_file");
	parseRegions("regions_file", &regionStart, &regionEnd);
//	dump();
	dumpMemoryMapping();

	/* init libtiming_papi */
	initMeasurement();
	printResultsHeader();

	/* start exemplar measurements */

//	startMeasurement();
//	stopMeasurement();
//	printResults(0);
//
//	startMeasurement();
//	stopMeasurement();
//	printResults(0);

	startMeasurement();
	doUnwind(0x400500, NULL/*context*/, &_flushToDiskBuffer[numberOfBufferElements]);
	stopMeasurement();
	printResults(0);

	numberOfBufferElements++;

	startMeasurement();
	doUnwind(0x400500, NULL/*context*/, &_flushToDiskBuffer[numberOfBufferElements]);
	stopMeasurement();
	printResults(0);

	numberOfBufferElements++;

	startMeasurement();
	doUnwind(0x400500, NULL/*context*/, &_flushToDiskBuffer[numberOfBufferElements]);
	stopMeasurement();
	printResults(0);

	return 0;
}
