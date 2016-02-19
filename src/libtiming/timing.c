#include "timing.h"
#include <stdlib.h>

static struct timespec start, end;

void initMeasurement() {}
void finalizeMeasurement() {}

void startMeasurement() {
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void stopMeasurement() {
	clock_gettime(CLOCK_MONOTONIC, &end);
}

int timespec_subtract(struct timespec *result, struct timespec *start, struct timespec *end) {
	if ((end->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = end->tv_sec - start->tv_sec - 1;
		result->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
		return -1;
	} else {
		result->tv_sec = end->tv_sec - start->tv_sec;
		result->tv_nsec = end->tv_nsec - start->tv_nsec;
		return 0;
	}
}

double getResult() {
	char resultString[32];
	struct timespec result;
	timespec_subtract(&result, &start, &end);
	sprintf(resultString, "%ld.%09ld", result.tv_sec, result.tv_nsec);

	char* stringEnd;
	return strtod(resultString, &stringEnd);
}

void printResultsHeader() {}
void printResults(const char* name) {
	struct timespec result;
	timespec_subtract(&result, &start, &end);
	fprintf(stdout, "%17s | %ld.%09ld seconds\n", name, result.tv_sec, result.tv_nsec);
	start.tv_sec = 0;
	start.tv_nsec = 0;
	end.tv_sec = 0;
	end.tv_nsec = 0;
}

