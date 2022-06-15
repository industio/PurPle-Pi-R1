#include <linux/sched.h>
#include <asm/stacktrace.h>

// check for 4.6 patch which changed the function signature of
// stacktrace_ops 'address' member.

int print_stack_address(void *data __attribute__ ((unused)),
			unsigned long addr __attribute__ ((unused)),
			int reliable __attribute__ ((unused)))
{
    return 0;
}

struct stacktrace_ops ops __attribute__ ((unused)) = {.address=print_stack_address};
