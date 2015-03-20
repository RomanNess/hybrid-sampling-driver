#include "unwinding.h"

#define PRINT_FUNCTIONS 1
#define IGNORE_PAPI_CONTEXT 0

// XXX RN: this only ategorizes functions of the target binary (not linked libs) as interesting
// XXX RN: note that the SPs are currently not used
// XXX RN: monitor_get_addr_main() can obtain the address of main() for error handling
/* context - the papi handler context
 * Returns the address of the first "interesting" function (-1 if no interesting function found).
 *
 * Note: unw_step returns
 * 		>0 for successful unwinds
 * 		<0 error
 */
long doUnwind(unsigned long address, void* context, struct SampleEvent *buffer) {

	unw_cursor_t cursor;
#if IGNORE_PAPI_CONTEXT
	unw_context_t uc;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
#else
	/* RN: I have no idea why exactly this works with papi overflow contexts but it does! */
	unw_context_t* uc = (unw_context_t*) context;
	unw_init_local(&cursor, uc);
#endif

#if  PRINT_FUNCTIONS
	printf("Sample: %li\n", sampleCount);
	unw_word_t offp;
	char buf[512];
#endif

	// unwind till first interesting function
	unsigned long functionAddress = address;
	while (functionAddress < regionStart || functionAddress > regionEnd) {
#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("ip = %lx \t| %s (SKIP)\n", functionAddress, buf);
#endif
		if (unw_step(&cursor) < 0) {
			buffer->numUnwindEvents = 0;
#if PRINT_FUNCTIONS
			printf("### skipped unwind method.\n\n");
#endif
			return -1;
		}
		unw_get_reg(&cursor, UNW_REG_IP, &functionAddress);
	}

	functionAddress = getFunctionStartAddress(functionAddress);

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("Starting Function: ip = %lx \t| %s \n", (unsigned long) functionAddress, buf);
#endif

	int unwindSteps = getUnwindSteps(functionAddress);
	buffer->numUnwindEvents = unwindSteps;
	buffer->unwindEvents = (struct StackEvent *) malloc(unwindSteps * sizeof(struct StackEvent));

	unsigned long ip, sp;
	int status = 1;
	while (status > 0 && unwindSteps != 0) {

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		if (regionStart < ip && ip < regionEnd) {

			unwindSteps--;	// interesting frame

			buffer->unwindEvents[unwindSteps].identifier = ip;

#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
			printf("ip = %lx \t| sp = %lx | %s | %i\n", (unsigned long) ip, (unsigned long) sp, buf, unwindSteps);
#endif
		} else {
#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
			printf("ip = %lx \t| sp = %lx | %s | %i (IGNORE)\n", (unsigned long) ip, (unsigned long) sp, buf, unwindSteps);
#endif
		}

		status = unw_step(&cursor);
	}

#if  PRINT_FUNCTIONS
	printf("\n");
#endif

	return (long int) functionAddress;
}
