#include <asm/delay.h>

void foo (unsigned long long usecs)
{
    udelay_simple(usecs);
}
