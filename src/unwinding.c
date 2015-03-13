#include "unwinding.h"

#define PRINT_FUNCTIONS 1

// TODO use unwindSteps
// TODO how to distinguish methods of our library and actual target code during unwind ???
// TODO where to parse function identifiers for the runtime?
/* context - the papi handler context */
void doUnwind(unsigned long address, void* context) {

	unw_cursor_t cursor;
	/* RN: I have no idea why exactly this works with papi overflow contexts but it does! */
	unw_context_t* uc = (unw_context_t*) context;
	unw_init_local(&cursor, uc);

	unsigned long unwindBuffer[MAX_UNWIND_FACTOR];
	unsigned long* bufferTop = unwindBuffer;

	// for debug output
	unw_word_t offp;
	char buf[512];

	// unwind till first interesting function
	unsigned long functionStart = getFunctionStart((unsigned long) address);
	while (functionStart == 0 || functionStart < regionStart || functionStart > regionEnd) {

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("ip = %lx \t| %s (ignored)\n", (unsigned long) functionStart, buf);
#endif
		unw_step(&cursor);
		unw_get_reg(&cursor, UNW_REG_IP, &functionStart);
	}

	int unwindSteps = getUnwindSteps(getFunctionStart(functionStart));

	unsigned long ip, sp;
	int status = 1;
	unsigned long bot = (unsigned long) monitor_stack_bottom();
	while (status > 0 && unwindSteps != 0) {

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		if (regionStart < ip && ip < regionEnd) {

			*bufferTop = ip;
			bufferTop++;
			unwindSteps--;	// interesting frame

#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
			printf("ip = %lx \t| sp = %lx | %s | %i\n", (unsigned long) ip, (unsigned long) sp, buf, unwindSteps);
#endif
		} else {
#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
			printf("ip = %lx \t| sp = %lx | %s | %i (ignored)\n", (unsigned long) ip, (unsigned long) sp, buf, unwindSteps);
#endif
		}

		status = unw_step(&cursor);

	}

	///XXX
	printf("stack bottom: %lx\n", (unsigned long) bot);
	printf("\n");

}
