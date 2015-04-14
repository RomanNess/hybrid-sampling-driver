#include "papi.h"
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

static int EventSet = PAPI_NULL;
static long_long values[10];
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
	// CI: Added PAPI to measure number of cycles
	int retval;
	elg_cycles_per_sec = elg_pform_cpuinfo();
	/* Initialize the library */
	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT && retval > 0) {
		fprintf(stderr, "PAPI library version mismatch!\en");
		exit(1);
	}
//PAPI_BR_UCN PAPI_BR_INS PAPI_TOT_CYC PAPI_REF_CYC
	if ((PAPI_create_eventset(&EventSet) != PAPI_OK))
	{
		fprintf(stderr, "Error initializing the event set!!\en");
		exit(1);
	}
	if ((PAPI_add_event(EventSet, PAPI_TOT_INS) != PAPI_OK))
	{
		fprintf(stderr, "Error adding set PAPI_TOT_INS\n");
		exit(1);
	}
	if ((PAPI_add_event(EventSet, PAPI_TOT_CYC) != PAPI_OK))
	{
		fprintf(stderr, "Error adding set PAPI_TOT_CYC\n");
		exit(1);
	}
	if ((PAPI_add_event(EventSet, PAPI_REF_CYC) != PAPI_OK))
	{
		fprintf(stderr, "Error adding set PAPI_REF_CYC\n");
		exit(1);
	}
	if ((PAPI_add_event(EventSet, PAPI_BR_UCN) != PAPI_OK))
	{
		fprintf(stderr, "Error adding set PAPI_BR_UCN\n");
		exit(1);
	}
	if ((PAPI_add_event(EventSet, PAPI_BR_INS) != PAPI_OK))
	{
		fprintf(stderr, "Error adding set PAPI_BR_INS\n");
		exit(1);
	}
	// XXX RN: it seems like this does not fit in the eventset on hla login nodes
//	if ((PAPI_add_event(EventSet, PAPI_FP_INS) != PAPI_OK))
//	{
//		fprintf(stderr, "Error adding set PAPI_FP_INS\n");
//		exit(1);
//	}
}
double startInS, stopInS;
void startMeasurement() {
	PAPI_start(EventSet);

	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));
	uint64_t clock_value;
	clock_value = ((uint64_t) high << 32) + low;
	startInS = (double) (clock_value) / (double) elg_cycles_per_sec;
}

void stopMeasurement() {
	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));
	uint64_t clock_value;
	clock_value = ((uint64_t) high << 32) + low;
	stopInS = (double) (clock_value) / (double) elg_cycles_per_sec;

	if (PAPI_stop(EventSet, values) != PAPI_OK) {
		fprintf(stderr, "Error reading counters\n");
		exit(1);
	}
}

void finalizeMeasurement() {
	PAPI_cleanup_eventset(EventSet);
	PAPI_destroy_eventset(&EventSet);
}

void printResultsHeader()
{
	fprintf(stderr,
			"\tName |\t Runtime in s|\tPAPI_TOT_INS |\tPAPI_TOT_CYC |\tPAPI_REF_CYC |\tPAPI_BR_UCN |\tPAPI_BR_INS |\tPAPI_FP_INS\n");
}

void printResults(const char* name) {
//	fprintf(stderr,"PAPI_TOT_INS = %lli (%lli / iterationr)\n",values[0],values[0]/iterations);
//	fprintf(stderr,"PAPI_TOT_CYC = %lli (%lli / iterationr)\n",values[1],values[1]/iterations);
//	fprintf(stderr,"PAPI_REF_CYC = %lli (%lli / iterationr)\n",values[2],values[2]/iterations);
//	fprintf(stderr,"PAPI_BR_UCN  = %lli (%lli / iterationr)\n",values[3],values[3]/iterations);
//	fprintf(stderr,"PAPI_BR_INS  = %lli (%lli / iterationr)\n",values[4],values[4]/iterations);
//	fprintf(stderr,"PAPI_FP_INS  = %lli (%lli / iterationr)\n",values[5],values[5]/iterations);
	fprintf(stderr, "%12s |\t%10.9lf s|\t%12lli |\t%12lli |\t%12lli |\t%11lli |\t%11lli |\t%11lli\n", name,
			stopInS - startInS, values[0], values[1], values[2], values[3], values[4], values[5]);
}

#ifdef __cplusplus
}
#endif 
