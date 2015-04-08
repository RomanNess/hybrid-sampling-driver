
#include "time.h"
#include "stdio.h"
#ifdef __cplusplus
extern "C" {
#endif
int timespec_subtract(struct timespec* result, struct timespec* start, struct timespec* end);

//struct timespec start;
//struct timespec end;


void startMeasurement();
void stopMeasurement();
void printResults(int iterations);
void finalizeMeasurement();
void initMeasurement();
void printResultsHeader();
#ifdef __cplusplus
}
#endif
