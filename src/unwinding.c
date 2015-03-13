#include "unwinding.h"

#define PRINT_FUNCTIONS 1

// TODO use unwindSteps
// TODO how to distinguish methods of our library and actual target code during unwind ???
// TODO where to parse function identifiers for the runtime?
/* context - the papi handler context */
void doUnwind(int address, void* context) {
	unw_cursor_t cursor;
	/* RN: I have no idea why exactly this works with papi overflow contexts but it does! */
	unw_context_t* uc = (unw_context_t*) context;
	unw_init_local(&cursor, uc);

	unw_cursor_t unwindBuffer[MAX_UNWIND_FACTOR];
	unw_cursor_t* bufferTop = unwindBuffer;

			// for debug output
			unw_word_t offp;
			char buf[512];

	// unwind till first interesting function
	unsigned long functionStart = getFunctionStart( (unsigned long) address);
	while (functionStart == 0 || functionStart < regionStart || functionStart > regionEnd) {

#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof (buf), &offp);
		printf ("ip = %x \t| %s (ignored)\n", (unsigned int) functionStart, buf);
#endif
		unw_step(&cursor);
		unw_get_reg(&cursor, UNW_REG_IP, &functionStart);
	}

	int unwindSteps = getUnwindSteps(getFunctionStart(functionStart));

	unsigned long ip, sp;
	int status = 1;
	unsigned long bot = (unsigned long) monitor_stack_bottom();
	while ( status > 0 && unwindSteps != 0 ) {
		*bufferTop = cursor;
		bufferTop++;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		if (regionStart<ip && ip<regionEnd) {
			unwindSteps--;	// interesting frame
#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof (buf), &offp);
			printf ("ip = %x \t| sp = %x | %s | %i\n", (unsigned int) ip, (unsigned int) sp, buf, unwindSteps);
#endif
		} else {
#if  PRINT_FUNCTIONS
			unw_get_proc_name(&cursor, buf, sizeof (buf), &offp);
			printf ("ip = %x \t| sp = %x | %s | %i (ignored)\n", (unsigned int) ip, (unsigned int) sp, buf, unwindSteps);
#endif
		}

		status = unw_step(&cursor);

	}

	///XXX
	printf("stack bottom: %x\n", (unsigned int) bot);
	printf("\n");

}
