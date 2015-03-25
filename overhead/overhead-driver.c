#include <../libtiming/timing.h>

#include <stdio.h>
#include <err.h>

#include <papi.h>

#include <libunwind.h>

#define NUM_ITERATIONS 1000000000
//#define NUM_ITERATIONS 10000000

long overflowCountForSamples = 2600000;
long sampleCount = 0;
int EventSet = PAPI_NULL;

void emptyHandler(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;
}

void unwindNewContext(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;

	unw_cursor_t cursor;
	unw_context_t uc;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
}

void unwindPAPIContext(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;

	unw_cursor_t cursor;
	unw_context_t* uc = (unw_context_t*) context;
	unw_init_local(&cursor, uc);
}

void unwindPAPIContextManual(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;

	unw_cursor_t cursor;
	unw_context_t uc;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

#if VERBOSE_PRINT
	unw_word_t offp;
	char buf[512];
	unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
	printf("\t%s\n", buf);
#endif
	// XXX RN: if libmonitor is active one more unwind (5) is necessary
	for (int i = 0; i < 4; i++) {
		unw_step(&cursor);

#if VERBOSE_PRINT
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("\t%s\n", buf);
#endif
	}
#if VERBOSE_PRINT
	printf("\n");
#endif
}

#include "cpp/hash.h"

void unwind(int EventSet, void* address, long long overflow_vector, void* context) {
	sampleCount++;

	getFunctionStartAddress((unsigned long) address);
}

void initPAPI(PAPI_overflow_handler_t handler) {

	EventSet = PAPI_NULL;
	int retval;
	/* start the PAPI Library */
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		errx(retval, "PAPI_library_init failed with %i", retval);
	}
	if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) {
		errx(retval, "PAPI_create_eventset failed with %i", retval);
	}
	if ((retval = PAPI_add_event(EventSet, PAPI_TOT_CYC)) != PAPI_OK) {
		errx(retval, "PAPI_create_eventset failed with %i", retval);
	}
	if ((retval = PAPI_overflow(EventSet, PAPI_TOT_CYC, overflowCountForSamples, 0, handler)) != PAPI_OK) {
		errx(retval, "PAPI_overflow failed with %i", retval);
	}
	if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
		errx(retval, "PAPI_start failed with %i", retval);
	}

}

void finiPAPI() {
	long long instructionCounter;
	PAPI_stop(EventSet, &instructionCounter);

	printf("%li samples taken == ", sampleCount);
	sampleCount = 0;
}

void kernel() __attribute__ ((noinline));
void kernel() {
	double f = 0.f;
	for (int i = 1; i < NUM_ITERATIONS; i++) {
		f = f + 1 / (double) i;
	}
	printf("", f);
}

void makeRun(PAPI_overflow_handler_t handler, const char* name) {

	printf("%17s == ", name);

	if (handler != NULL) {
		initPAPI(handler);
	}
	startMeasurement();
	kernel();
	stopMeasurement();
	if (handler != NULL) {
		finiPAPI();
	}
	printResults();
}

int main() {

	makeRun(NULL, "Reference");
	makeRun(NULL, "Reference");
	makeRun(NULL, "Reference");

	makeRun(emptyHandler, "Empty");
	makeRun(emptyHandler, "Empty");
	makeRun(emptyHandler, "Empty");

	makeRun(unwindNewContext, "UnwindNewContext");
	makeRun(unwindNewContext, "UnwindNewContext");
	makeRun(unwindNewContext, "UnwindNewContext");

	makeRun(unwindPAPIContext, "UnwindPAPIContext");
	makeRun(unwindPAPIContext, "UnwindPAPIContext");
	makeRun(unwindPAPIContext, "UnwindPAPIContext");

	makeRun(unwindPAPIContextManual, "UnwindPAPIManual");
	makeRun(unwindPAPIContextManual, "UnwindPAPIManual");
	makeRun(unwindPAPIContextManual, "UnwindPAPIManual");

	return 0;
}
