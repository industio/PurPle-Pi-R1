#include <linux/sched.h>

// On 4.13 kernels, linux/sched.h doesn't include
// linux/sched/clock.h, where cpu_clock() is declared.
#include <linux/blkdev.h>

//
// The following kernel commit inlined the cpu_clock()
//
// commit 2c923e94cd9c6acff3b22f0ae29cfe65e2658b40
// Author: Daniel Lezcano <daniel.lezcano@linaro.org>
// Date:   Mon Apr 11 16:38:34 2016 +0200
//
//     sched/clock: Make local_clock()/cpu_clock() inline
//
//     The local_clock/cpu_clock functions were changed to prevent a double
//     identical test with sched_clock_cpu() when HAVE_UNSTABLE_SCHED_CLOCK
//     is set. That resulted in one line functions.
//
//     As these functions are in all the cases one line functions and in the
//     hot path, it is useful to specify them as static inline in order to
//     give a strong hint to the compiler.
//
//     After verification, it appears the compiler does not inline them
//     without this hint. Change those functions to static inline.
//
//     sched_clock_cpu() is called via the inlined local_clock()/cpu_clock()
//     functions from sched.h. So any module code including sched.h will
//     reference sched_clock_cpu(). Thus it must be exported with the
//     EXPORT_SYMBOL_GPL macro.
//

void foo (void) {
   (void) cpu_clock(0);
}
