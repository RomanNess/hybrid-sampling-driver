
#include "../libtiming_papi/timing.h"

#include "driver.h"

#include <monitor.h>
#include <libunwind.h>

#include <stdio.h>

int calldepth = 0;

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	calldepth++;

	startMeasurement();

	// driver code here
	unw_cursor_t cursor;
	unw_context_t uc;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	int unwindSteps = calldepth;

	int status = 1;
	while (status > 0 && unwindSteps != 0) {
		unwindSteps--;
		status = unw_step(&cursor);
	}

	stopMeasurement();
	printResults("enter");
}

void __cyg_profile_func_exit(void *func_ptr, void *call_site) {
	calldepth--;

	return;
}

void *monitor_init_process(int *argc, char **argv, void *data) {

//	initBuffer();

	initMeasurement();
	printResultsHeader();

	return NULL;
}

void monitor_fini_process(int how, void* data) {

}

int main() {

	return 0;
}
