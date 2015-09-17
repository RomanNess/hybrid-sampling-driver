#ifndef SRC_TIMING_H
#define SRC_TIMING_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int timespec_subtract(struct timespec* result, struct timespec* start, struct timespec* end);

struct timespec start;
struct timespec end;

void startMeasurement();
void stopMeasurement();
void printResults();

#endif	// SRC_TIMING_H
