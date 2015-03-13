void _instroPushIdentifier(unsigned long funcIdentifier);
void _instroPopIdentifier();

void __cyg_profile_func_enter(void *func_ptr, void *call_site);
void __cyg_profile_func_exit(void *func_ptr, void *call_site);

void printBenchmarkResult() {
}

void _instroPushIdentifier(unsigned long funcIdentifier) {
	// Nothings happens here.
	return;
}

void _instroPopIdentifier() {
	// honthing happens here.
	return;
}

void __cyg_profile_func_enter(void *func_ptr, void *call_site) {
	return;
}

void __cyg_profile_func_exit(void *func_ptr, void *call_site) {
	return;
}
