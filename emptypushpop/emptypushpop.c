void _instroPopIdentifier (unsigned long long threadIdentifier);
void _instroPushIdentifier (unsigned long long funcIdentifier, unsigned long long threadIdentifier);

void printBenchmarkResult(){}

void _instroPushIdentifier(unsigned long long funcIdentifier, unsigned long long threadIdentifier){
	// Nothings happens here.
	return;
}

void _instroPopIdentifier(unsigned long long threadIdentifier){
	// honthing happens here.
	return;
}
