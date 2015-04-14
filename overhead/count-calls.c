#include <monitor.h>
#include <stdio.h>

static long numberOfCalls = 0;

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	numberOfCalls++;
}

void __cyg_profile_func_exit(void* func_ptr, void* call_site) {
}

void *monitor_init_process(int* argc, char** argv, void* data) {
	return NULL;
}

void monitor_fini_process(int how, void* data) {
	printf("Instrumented calls: %li\n", numberOfCalls);
}

