#include "papi.h"
#include "timing.h"
#include <stdlib.h>
#include <stdint.h>

#ifndef ELG_CPUFREQ
#  define ELG_CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#endif
#ifndef ELG_PROCDIR
#  define ELG_PROCDIR "/proc/"
#endif
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif
static int EventSet=PAPI_NULL;;
static long_long values[10];
static struct timespec start;
static struct timespec end;
static uint64_t       elg_cycles_per_sec=1;

static unsigned char elg_cpu_has_tsc=0;
static unsigned int  elg_cpu_count=0;


static uint64_t elg_pform_cpuinfo()
{
  FILE   *cpuinfofp;
  char   line[1024];
  char   *token;

  /* size corresponding to elg_cycles_per_sec */
  uint64_t hz = 0;

  /* check special file */
  if ((cpuinfofp = fopen (ELG_CPUFREQ, "r")) && fgets(line, sizeof(line), cpuinfofp)) {
    fclose(cpuinfofp);
    hz = 1000*atoll(line);
    return hz;
  }

  /* the goal is to run it on Juropa and /proc/cpuinfo exists there */
  cpuinfofp = fopen (ELG_PROCDIR "cpuinfo", "r");

  while (fgets(line, sizeof (line), cpuinfofp))
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
      while ( (token = (char*) strtok(NULL, " \n"))!=NULL)
      {
        if (strcmp(token,"tsc")==0)
        {
          elg_cpu_has_tsc=1;
        } else if (strcmp(token,"constant_tsc")==0)
        {
          elg_cpu_has_tsc=2;
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
	elg_cycles_per_sec=elg_pform_cpuinfo();
	/* Initialize the library */
	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT && retval > 0) {
		fprintf(stderr,"PAPI library version mismatch!\en");
		exit(1); 
	}
//PAPI_BR_UCN PAPI_BR_INS PAPI_TOT_CYC PAPI_REF_CYC
	if (	(PAPI_create_eventset(&EventSet) != PAPI_OK) )
	{
		fprintf(stderr,"Error initializing the event set!!\en");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_TOT_INS) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_TOT_INS\n");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_TOT_CYC) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_TOT_CYC\n");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_REF_CYC) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_REF_CYC\n");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_BR_UCN) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_BR_UCN\n");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_BR_INS) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_BR_INS\n");
		exit(1);
	}
	if (	(PAPI_add_event(EventSet, PAPI_FP_INS) != PAPI_OK) )
	{
		fprintf(stderr,"Error adding set PAPI_BR_INS\n");
		exit(1);
	}
}
double startInS,stopInS;
void startMeasurement(){

	PAPI_start(EventSet);
//	clock_gettime(CLOCK_MONOTONIC, &start);
	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));
        uint64_t clock_value;
	clock_value = ((uint64_t)high << 32) + low;
	startInS=(double) (clock_value) / (double) elg_cycles_per_sec;
}


void stopMeasurement(){
	uint32_t low = 0;
	uint32_t high = 0;
	asm volatile ("rdtscp" : "=a" (low), "=d" (high));
        uint64_t clock_value;
	clock_value = ((uint64_t)high << 32) + low;
	stopInS=(double) (clock_value) / (double) elg_cycles_per_sec;
//	clock_gettime(CLOCK_MONOTONIC, &end);
	if (PAPI_stop(EventSet, values) != PAPI_OK){
		fprintf(stderr,"Error reading counters\n");
		exit(1);
	}
	
}

void finalizeMeasurement(){
	PAPI_cleanup_eventset(EventSet);
	PAPI_destroy_eventset(&EventSet);
}

void printResultsHeader()
{
#ifdef AVERAGING
	fprintf(stderr,"Runtime(s)\tRuntime/Iteration(ns)\tPAPI_TOT_INS\tPAPI_TOT_INS/Iteration\tPAPI_TOT_CYC\tPAPI_TOT_CYC/Iteration\tPAPI_REF_CYC\tPAPI_REF_CYC/Iteration\tPAPI_BR_UCN\tPAPI_BR_UCN/Iteration\tPAPI_BR_INS\tPAPI_BR_INS/Iteration\tPAPI_FP_INS\tPAPI_FP_INS/iteration\n");
#else
	fprintf(stderr,";\tIterations ;\t Runtime in s ;\tPAPI_TOT_INS ;\tPAPI_TOT_CYC ;\tPAPI_REF_CYC ;\tPAPI_BR_UCN ;\tPAPI_BR_INS ;\tPAPI_FP_INS ;\n");
#endif
}
void printResults(int iteration){
	double iterations=iteration;
///	struct timespec result;
//	timespec_subtract(&result, &start, &end);
//	double nanoseconds=result.tv_sec/10e9+result.tv_nsec;
//	fprintf(stdout, " %lld.%lld seconds (%lf ns / iteration).\n", result.tv_sec, result.tv_nsec,nanoseconds/iterations);
//	fprintf(stdout, "LIBTIMING: Run took: %lld.%lld seconds (%lf ns / iteration).\n", result.tv_sec, result.tv_nsec,nanoseconds/iterations);
//	start.tv_sec=0;
//	start.tv_nsec=0;
//	end.tv_sec= 0;
//	end.tv_nsec=0;
//	fprintf(stderr,"PAPI_TOT_INS = %lli (%lli / iterationr)\n",values[0],values[0]/iterations);
//	fprintf(stderr,"PAPI_TOT_CYC = %lli (%lli / iterationr)\n",values[1],values[1]/iterations);
//	fprintf(stderr,"PAPI_REF_CYC = %lli (%lli / iterationr)\n",values[2],values[2]/iterations);
//	fprintf(stderr,"PAPI_BR_UCN  = %lli (%lli / iterationr)\n",values[3],values[3]/iterations);
//	fprintf(stderr,"PAPI_BR_INS  = %lli (%lli / iterationr)\n",values[4],values[4]/iterations);
//	fprintf(stderr,"PAPI_FP_INS  = %lli (%lli / iterationr)\n",values[5],values[5]/iterations);
#ifdef AVERAGING
	if (iteration==0) terations=iteration=1;
	fprintf(stderr, ";\t%lld.%.9lld s;\t %lf ns;\t%8lli;\t%4lf;\t%8lli;\t%4lf;\t%8lli;\t%4lf;\t%8lli;\t%4lf;\t%8lli;\t%4lf;\t%8lli;\t%4lf;.\n", result.tv_sec, result.tv_nsec,nanoseconds/iterations,values[0],values[0]/iterations,values[1],values[1]/iterations,values[2],values[2]/iterations,values[3],values[3]/iterations,values[4],values[4]/iterations,values[5],values[5]/iterations);
#else
	fprintf(stderr, ";\t%10li ;\t%10.9lf s;\t%12lli ;\t%12lli ;\t%12lli ;\t%11lli ;\t%11lli ;\t%11lli ;\n", iteration,stopInS-startInS,values[0],values[1],values[2],values[3],values[4],values[5]);
#endif
//	fprintf(stderr,"%.9lf\n",stopInS-startInS);
}


int timespec_subtract (struct timespec *result, struct timespec *start, struct timespec *end){
        if ((end->tv_nsec-start->tv_nsec)<0) {
                result->tv_sec = end->tv_sec - start->tv_sec - 1;
                result->tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
                return -1;
        } else {
                result->tv_sec = end->tv_sec-start->tv_sec;
                result->tv_nsec = end->tv_nsec-start->tv_nsec;
                return 0;
        }
}
#ifdef __cplusplus
}
#endif 
