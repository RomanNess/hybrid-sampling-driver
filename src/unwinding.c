#include "unwinding.h"

#define PRINT_FUNCTIONS 1

// TODO how to distinguish methods of our library and actual target code during unwind ???
/* context - the papi handler context */
void doUnwind(unsigned long address, void* context, struct SampleEvent *buffer) {

	unw_cursor_t cursor;
	/* RN: I have no idea why exactly this works with papi overflow contexts but it does! */
	unw_context_t* uc = (unw_context_t*) context;
	unw_init_local(&cursor, uc);

#if  PRINT_FUNCTIONS
	// for debug output
	unw_word_t offp;
	char buf[512];
#endif
	// unwind till first interesting function
	int status = 1;
	unsigned long functionStart = getFunctionStart((unsigned long) address);
	while (functionStart == 0 || functionStart < regionStart || functionStart > regionEnd) {
#if PRINT_FUNCTIONS
		unw_get_proc_name(&cursor, buf, sizeof(buf), &offp);
		printf("ip = %lx \t| %s (SKIP)\n", (unsigned long) functionStart, buf);
#endif
		if (unw_step(&cursor) < 0) {
			printf("### skipped unwind method.\n\n");
			return;
		}
		unw_get_reg(&cursor, UNW_REG_IP, &functionStart);
	}

	int unwindSteps = getUnwindSteps(getFunctionStart(functionStart));
	buffer->numUnwindEvents = unwindSteps;
	buffer->unwindEvents = (struct StackEvent *) malloc(unwindSteps * sizeof(struct StackEvent));

	unsigned long ip, sp;
	unsigned long bot = (unsigned long) monitor_stack_bottom();
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
	///XXX
	printf("stack bottom: %lx\n", (unsigned long) bot);
	printf("\n");
#endif
}
