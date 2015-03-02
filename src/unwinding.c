#include "unwinding.h"

// TODO use unwindSteps
// TODO how to distinguish methods of our library and actual target code during unwind ???
// TODO where to parse function identifiers for the runtime?
void doUnwind(int unwindSteps) {
	unw_cursor_t cursor;
	unw_context_t uc;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	unw_cursor_t unwindBuffer[MAX_UNWIND_FACTOR];
	unw_cursor_t* bufferTop = unwindBuffer;

	unsigned long ip, sp;
	int status = 1;
	unsigned long bot = (unsigned long) monitor_stack_bottom();
	while ( status > 0 ) {
//	while ( (long) sp !=  bot) {
		*bufferTop = cursor;
		bufferTop++;

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);

		// debug output
		unw_word_t offp;
		char buf[512];
		unw_get_proc_name(&cursor, buf, sizeof (buf), &offp);
		printf ("status: %i, ip = %x, sp = %x, %s\n", status, (unsigned int) ip, (unsigned int) sp, buf);

		status = unw_step(&cursor);
	}

	///XXX
	printf("stack bottom: %x\n", (unsigned int) bot);

	printf("\n");



}
