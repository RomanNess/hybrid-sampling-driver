#ifdef META_BENCHMARK
#include "libtiming_papi/timing.h"
#endif

void _instroPushIdentifier(unsigned long funcIdentifier);
void _instroPopIdentifier();

void __cyg_profile_func_enter(void *func_ptr, void *call_site);
void __cyg_profile_func_exit(void *func_ptr, void *call_site);

void printBenchmarkResult() {
}

void _instroPushIdentifier(unsigned long funcIdentifier) {
	return;
}

void _instroPopIdentifier() {
	return;
}

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	return;
}

void __cyg_profile_func_exit(void *func_ptr, void *call_site) {
	return;
}

#ifndef NO_INIT

void *monitor_init_process(int *argc, char **argv, void *data) {
#ifdef META_BENCHMARK
	initMeasurement();
	printResultsHeader();
	// warmup
	startMeasurement();
	stopMeasurement();
//	printResults("warmup");
	startMeasurement();
	stopMeasurement();
//	printResults("warmup");
	startMeasurement();
	stopMeasurement();
//	printResults("warmup");

	startMeasurement();
#endif

	return 0;
}

void monitor_fini_process(int how, void* data) {
#ifdef META_BENCHMARK
	stopMeasurement();
	printResults("target");
#endif
}

#endif // NO_INIT
