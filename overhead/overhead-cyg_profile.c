
#include "../libtiming_papi/timing.h"

#include "../src/driver.h"

#include <monitor.h>

int calldepth = 0;

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	calldepth++;

	startMeasurement();

	// driver code here

	stopMeasurement();
	printResults(0);
}

void __cyg_profile_func_exit(void *func_ptr, void *call_site) {
	calldepth--;

	return;
}

void *monitor_init_process(int *argc, char **argv, void *data) {

//	initBuffer();

	initMeasurement();

	return NULL;
}

void monitor_fini_process(int how, void* data) {

}

//int main() {
//
//	startMeasurement();
//	doUnwind(0x400500, NULL/*context*/, &_flushToDiskBuffer[numberOfBufferElements]);
//	stopMeasurement();
//	printResults(0);
//
//	return 0;
//}
