void _instroPopIdentifier (unsigned long long threadIdentifier);
void _instroPushIdentifier (unsigned long long funcIdentifier, unsigned long long threadIdentifier);

void __cyg_profile_func_enter(void *func_ptr, void *call_site);
void __cyg_profile_func_exit(void *func_ptr, void *call_site);


void printBenchmarkResult(){}

void _instroPushIdentifier(unsigned long long funcIdentifier, unsigned long long threadIdentifier){
        // Nothings happens here.
        return;
}

void _instroPopIdentifier(unsigned long long threadIdentifier){
        // honthing happens here.
        return;
}


void __cyg_profile_func_enter(void *func_ptr, void *call_site){
    return;
}

void __cyg_profile_func_exit(void *func_ptr, void *call_site){
    return;
}
