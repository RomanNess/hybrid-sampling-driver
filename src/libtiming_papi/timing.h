#ifndef SRC_TIMING_H_
#define SRC_TIMING_H_

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
void printResults(const char* name);

#ifdef __cplusplus
}
#endif

#endif	// SRC_TIMING_H_
