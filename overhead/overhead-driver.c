#include <libtiming/timing.h>

#include <stdio.h>
#include <err.h>

#include <papi.h>

#include <libunwind.h>

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

	for (int i=0; i<6; i++) {
		unw_step(&cursor);
	}
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

	printf("%li samples taken\n", sampleCount);
	sampleCount = 0;
}

void kernel() __attribute__ ((noinline));
void kernel() {
	double f = 0.f;
	for (int i = 1; i < 1000000000; i++) {
		f = f + 1 / (double) i;
	}
	printf("", f);
}

void makeRun(PAPI_overflow_handler_t handler, const char* name) {

	printf("\n%s ==\n", name);

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
	makeRun(emptyHandler, "Empty");
	makeRun(unwindNewContext, "UnwindNewContext");
	makeRun(unwindPAPIContext, "UnwindPAPIContext");
	makeRun(unwindPAPIContextManual, "UnwindPAPIContextManual");

	makeRun(NULL, "Reference");

	return 0;
}
