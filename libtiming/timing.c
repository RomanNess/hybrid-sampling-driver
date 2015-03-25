#include "timing.h"

void startMeasurement() {
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void stopMeasurement() {
	clock_gettime(CLOCK_MONOTONIC, &end);
}

double getResult() {
	char resultString[32];
	struct timespec result;
	timespec_subtract(&result, &start, &end);
	sprintf(resultString, "%ld.%09ld", result.tv_sec, result.tv_nsec);

	char* stringEnd;
	return strtod(resultString, &stringEnd);
}

void printResults() {
	struct timespec result;
	timespec_subtract(&result, &start, &end);
	fprintf(stderr, "Run took: %ld.%09ld seconds.\n", result.tv_sec, result.tv_nsec);
	start.tv_sec = 0;
	start.tv_nsec = 0;
	end.tv_sec = 0;
	end.tv_nsec = 0;
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

