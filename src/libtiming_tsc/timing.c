#include "timing.h"
#include <stdlib.h>
#include <stdint.h>
#include "string.h"

#ifndef ELG_CPUFREQ
#  define ELG_CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#endif
#ifndef ELG_PROCDIR
#  define ELG_PROCDIR "/proc/"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static uint64_t elg_cycles_per_sec = 1;

static unsigned char elg_cpu_has_tsc = 0;
static unsigned int elg_cpu_count = 0;

static uint64_t elg_pform_cpuinfo()
{
	FILE *cpuinfofp;
	char line[1024];
	char *token;

	/* size corresponding to elg_cycles_per_sec */
	uint64_t hz = 0;

	/* check special file */
	if ((cpuinfofp = fopen(ELG_CPUFREQ, "r")) && fgets(line, sizeof(line), cpuinfofp)) {
		fclose(cpuinfofp);
		hz = 1000 * atoll(line);
		return hz;
	}

	/* the goal is to run it on Juropa and /proc/cpuinfo exists there */
	cpuinfofp = fopen(ELG_PROCDIR "cpuinfo", "r");

	while (fgets(line, sizeof(line), cpuinfofp))
	{
		if (!strncmp("processor", line, 9))
			elg_cpu_count++;
#ifdef __ia64__
		if (!strncmp("itc MHz", line, 7))
#else
		if (!strncmp("cpu MHz", line, 7))
#endif
				{
			strtok(line, ":");

			hz = strtol((char*) strtok(NULL, " \n"), (char**) NULL, 0) * 1e6;
		}
		if (!strncmp("timebase", line, 8))
				{
			strtok(line, ":");

			hz = strtol((char*) strtok(NULL, " \n"), (char**) NULL, 0);
		}
		if (!strncmp("flags", line, 5))
				{
			strtok(line, ":");
			while ((token = (char*) strtok(NULL, " \n")) != NULL)
			{
				if (strcmp(token, "tsc") == 0)
						{
					elg_cpu_has_tsc = 1;
				} else if (strcmp(token, "constant_tsc") == 0)
						{
					elg_cpu_has_tsc = 2;
				}
			}
		}
	}

	fclose(cpuinfofp);
	return hz;
}

void initMeasurement()
{
	elg_cycles_per_sec = elg_pform_cpuinfo();
}

uint64_t startInS, stopInS;
void startMeasurement() {

	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));

//	asm volatile ("CPUID\n\t"
//	"RDTSC\n\t"
//	"mov %%edx, %0\n\t"
//	"mov %%eax, %1\n\t": "=r" (high), "=r" (low)::
//	"%rax", "%rbx", "%rcx", "%rdx");

	startInS = ((uint64_t) high << 32) + low;
}

void stopMeasurement() {
	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));

//	asm volatile("RDTSCP\n\t"
//	"mov %%edx,  %0\n\t"
//	"mov %%eax,  %1\n\t"
//	"CPUID\n\t": "=r" (high), "=r" (low)::
//	"%rax", "%rbx", "%rcx", "%rdx");

	stopInS = ((uint64_t) high << 32) + low;

}

void finalizeMeasurement() {
}

void printResultsHeader()
{
	fprintf(stderr,
			"cycles_per_sec= %lu \n" \
			"        Name |  Runtime in s| Raw Cycles | +Raw\n", elg_cycles_per_sec);
}

void printResults(const char* name) {
	uint64_t diff = stopInS - startInS;
	double timeDiff = (double) (diff) / (double) elg_cycles_per_sec;

	fprintf(stderr, "%12s | %10.9lf s| %11lu\n",
			name, timeDiff, diff);
}

#ifdef __cplusplus
}
#endif 
