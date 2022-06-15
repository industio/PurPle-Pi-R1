/* -*- linux-c -*-
 *
 * MIPS dwarf unwinder header file
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */
#ifndef _STP_ARM_UNWIND_H
#define _STP_ARM_UNWIND_H

#include <linux/sched.h>
#include <asm/ptrace.h>

#define _stp_get_unaligned(ptr) (*(ptr))

#define UNW_PC(frame)        (frame)->regs.cp0_epc
#define UNW_SP(frame)        (frame)->regs.regs[29] 

#define STACK_LIMIT(ptr)     (((ptr) - 1) & ~(THREAD_SIZE - 1))

#define UNW_REGISTER_INFO \
	PTREGS_INFO(regs[0]), \
	PTREGS_INFO(regs[1]), \
	PTREGS_INFO(regs[2]), \
	PTREGS_INFO(regs[3]), \
	PTREGS_INFO(regs[4]), \
	PTREGS_INFO(regs[5]), \
	PTREGS_INFO(regs[6]), \
	PTREGS_INFO(regs[7]), \
	PTREGS_INFO(regs[8]), \
	PTREGS_INFO(regs[9]), \
	PTREGS_INFO(regs[10]), \
	PTREGS_INFO(regs[11]), \
	PTREGS_INFO(regs[12]), \
	PTREGS_INFO(regs[13]), \
	PTREGS_INFO(regs[14]), \
	PTREGS_INFO(regs[16]), \
	PTREGS_INFO(regs[17]), \
	PTREGS_INFO(regs[18]), \
	PTREGS_INFO(regs[19]), \
	PTREGS_INFO(regs[20]), \
	PTREGS_INFO(regs[21]), \
	PTREGS_INFO(regs[22]), \
	PTREGS_INFO(regs[23]), \
	PTREGS_INFO(regs[24]), \
	PTREGS_INFO(regs[26]), \
	PTREGS_INFO(regs[27]), \
	PTREGS_INFO(regs[28]), \
	PTREGS_INFO(regs[29]), \
	PTREGS_INFO(regs[30]), \
	PTREGS_INFO(regs[31]), \
	PTREGS_INFO(lo), \
	PTREGS_INFO(hi), \
	PTREGS_INFO(cp0_epc)

#define UNW_PC_IDX 34
#define UNW_SP_IDX 29

/* Use default rules. The stack pointer should be set from the CFA.
   And the instruction pointer should be set from the return address
   column (which normally is the return register (regs[31]). */

static inline void arch_unw_init_frame_info(struct unwind_frame_info *info,
                                            /*const*/ struct pt_regs *regs,
					    int sanitize)
{
	if (&info->regs == regs) { /* happens when unwinding kernel->user */
		info->call_frame = 1;
		return;
	}

	memset(info, 0, sizeof(*info));
	/* XXX handle sanitize??? */
	info->regs = *regs;
}

#endif /* _STP_ARM_UNWIND_H */
