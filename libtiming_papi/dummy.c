#include <stdio.h>
unsigned long int enters;
unsigned long int exits;
static unsigned int initalized=0;
void __cyg_profile_func_enter (void *this_fn,
		void *call_site)
{
	enters++;
};
void __cyg_profile_func_exit  (void *this_fn,
		void *call_site)
{
	exits++;
};

void enter_function()
{
	enters++;
}

void exit_function()
{
	exits++;
}

void __attribute__((destructor)) dummyLibDone()
{
	if (initalized==1)
{
	printf("Enters %lu Exits %lu\n",enters,exits);
}
	initalized--;
};
void __attribute__((constructor)) dummyLibLoad()
{
	if (initalized==0)
{
	printf("Initializing Dummy Library!\n");
	enters=0;
	exits=0;
}
	initalized++;
};
