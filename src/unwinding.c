#include "unwinding.h"
#include "driver.h"

#define IGNORE_PAPI_CONTEXT 0
#define PRINT_FUNCTIONS 0
#define COUNT_UNW_STEPS() unwindStepsTaken++;
#define COUNT_PRE_UNW_STEPS() unwindStepsTakenPre++;


// XXX RN: this only categorizes functions of the target binary (not linked libs) as interesting
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
	unw_word_t offp;
	char buf[512];
#endif

	// unwind till first interesting function
	unsigned long oldFunctionAddress = 0;
	unsigned long functionAddress = address;
	while (functionAddress < targetRegionStart || functionAddress > targetRegionEnd) {
		oldFunctionAddress = functionAddress;
#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("ip = %lx \t| %s (SKIP)\n", functionAddress, buf);
#endif
		if (unw_step(&cursor) < 0) {

			COUNT_UNW_STEPS()
			COUNT_PRE_UNW_STEPS()

			buffer->numUnwindEvents = 0;
#if PRINT_FUNCTIONS
			printf("### skipped unwind method.\n\n");
#endif
			return -1L;
		}
		unw_get_reg(&cursor, UNW_REG_IP, &functionAddress);

		if (functionAddress == oldFunctionAddress) {
#if PRINT_FUNCTIONS
			printf("unwind got stuck and was skipped.\n");
#endif
			return -3L;	// can not unwind
		}
	}

	functionAddress = getFunctionStartAddress(functionAddress);
	int unwindSteps = getUnwindSteps(functionAddress);

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("Starting Function: ip = %lx \t| %s (%i)\n", (unsigned long) functionAddress, buf, unwindSteps);
#endif
	
	buffer->numUnwindEvents = unwindSteps;
	if (unwindSteps == 0) {
#if PRINT_FUNCTIONS
		printf("\n");
#endif
		return functionAddress;
	}

	buffer->unwindEvents = &_innerBuffer[_innerBufferSize];
	_innerBufferSize += unwindSteps;

	unsigned long ip = 0;
	oldFunctionAddress = 42;
	unsigned long sp;
	int status = 1;
	while (status > 0 && unwindSteps != 0) {

#if  PRINT_FUNCTIONS
	printf("loop.\n");
#endif


		if (mainRegionStart <= ip && ip <= mainRegionEnd) {
			break;
		}

		oldFunctionAddress = ip;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		if (ip == oldFunctionAddress) {
			return -2L;	// unwind stuck
		}

		if (targetRegionStart <= ip && ip <= targetRegionEnd) {

			// TODO count down after unwind
			status = unw_step(&cursor);
			unwindSteps--;	// interesting frame

			COUNT_UNW_STEPS()

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

	}

	#if  PRINT_FUNCTIONS
		printf("\n");
	#endif

	return (long int) functionAddress;
}
