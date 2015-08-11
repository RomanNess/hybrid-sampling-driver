
#include "../libtiming_papi/timing.h"

//#include "driver.h"

#include <monitor.h>
#include <libunwind.h>

#include <stdio.h>
#include <stdlib.h>

//#define NO_UNWIND
//#define UNW_INIT_ONLY
#define PRINT_FUNCTIONS 0

int calldepth = -1;

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	calldepth++;

	startMeasurement();

#ifndef NO_UNWIND
	// driver code here
	unw_cursor_t cursor;
	unw_context_t uc;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

#ifndef UNW_INIT_ONLY

	int unwindSteps = calldepth;

#if  PRINT_FUNCTIONS
	unw_word_t offp;
	char buf[512];
#endif

	int status = 1;
	while (status > 0 && unwindSteps != 0) {

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf(" %s\n", buf);
#endif

		unwindSteps--;
		status = unw_step(&cursor);
	}

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf(" %s\n", buf);
#endif

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

//	initBuffer();

	unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_NONE);

	initMeasurement();
	printResultsHeader();

	startMeasurement();
	stopMeasurement();
	printResults("warmup");
	startMeasurement();
	stopMeasurement();
	printResults("ref");

	return NULL;
}

void monitor_fini_process(int how, void* data) {
	finalizeMeasurement();
}

int main() {

	return 0;
}