
#include "time.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

void initMeasurement();
void finalizeMeasurement();

void startMeasurement();
void stopMeasurement();

void printResultsHeader();
void printResults(int iterations);

#ifdef __cplusplus
}
#endif
