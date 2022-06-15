/*
 *
 * Automatically generated file; DO NOT EDIT.
 * Linux/arm 4.9.84 Kernel Configuration
 *
 */
/*
 * The use of "&&" / "||" is limited in certain expressions.
 * The followings enable to calculate "and" / "or" with macro expansion only.
 */
/*
 * Helper macros to use CONFIG_ options in C/CPP expressions. Note that
 * these only work with boolean and tristate options.
 */
/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
/*
 * IS_BUILTIN(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y', 0
 * otherwise. For boolean options, this is equivalent to
 * IS_ENABLED(CONFIG_FOO).
 */
/*
 * IS_MODULE(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'm', 0
 * otherwise.
 */
/*
 * IS_REACHABLE(CONFIG_FOO) evaluates to 1 if the currently compiled
 * code can call a function defined in code compiled based on CONFIG_FOO.
 * This is similar to IS_ENABLED(), but returns false when invoked from
 * built-in code when CONFIG_FOO is set to 'm'.
 */
/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y' or 'm',
 * 0 otherwise.
 */
/* ld script to make ARM Linux kernel
 * taken from the i386 version by Russell King
 * Written by Martin Mares <mj@atrey.karlin.mff.cuni.cz>
 */
/*
 * Helper macros to support writing architecture specific
 * linker scripts.
 *
 * A minimal linker scripts has following content:
 * [This is a sample, architectures may have special requiriements]
 *
 * OUTPUT_FORMAT(...)
 * OUTPUT_ARCH(...)
 * ENTRY(...)
 * SECTIONS
 * {
 *	. = START;
 *	__init_begin = .;
 *	HEAD_TEXT_SECTION
 *	INIT_TEXT_SECTION(PAGE_SIZE)
 *	INIT_DATA_SECTION(...)
 *	PERCPU_SECTION(CACHELINE_SIZE)
 *	__init_end = .;
 *
 *	_stext = .;
 *	TEXT_SECTION = 0
 *	_etext = .;
 *
 *      _sdata = .;
 *	RO_DATA_SECTION(PAGE_SIZE)
 *	RW_DATA_SECTION(...)
 *	_edata = .;
 *
 *	EXCEPTION_TABLE(...)
 *	NOTES
 *
 *	BSS_SECTION(0, 0, 0)
 *	_end = .;
 *
 *	STABS_DEBUG
 *	DWARF_DEBUG
 *
 *	DISCARDS		// must be the last
 * }
 *
 * [__init_begin, __init_end] is the init section that may be freed after init
 * 	// __init_begin and __init_end should be page aligned, so that we can
 *	// free the whole .init memory
 * [_stext, _etext] is the text section
 * [_sdata, _edata] is the data section
 *
 * Some of the included output section have their own set of constants.
 * Examples are: [__initramfs_start, __initramfs_end] for initramfs and
 *               [__nosave_begin, __nosave_end] for the nosave data
 */
/*
 * Export symbols from the kernel to modules.  Forked from module.h
 * to reduce the amount of pointless cruft we feed to gcc when only
 * exporting a simple symbol or two.
 *
 * Try not to add #includes here.  It slows compilation and makes kernel
 * hackers place grumpy comments in header files.
 */
/* Some toolchains use a `_' prefix for all user symbols. */
/* Indirect, so macros are expanded before pasting. */
/* Align . to a 8 byte boundary equals to maximum function alignment. */
/*
 * LD_DEAD_CODE_DATA_ELIMINATION option enables -fdata-sections, which
 * generates .data.identifier sections, which need to be pulled in with
 * .data. We don't want to pull in .data..other sections, which Linux
 * has defined. Same for text and bss.
 */
/*
 * Align to a 32 byte boundary equal to the
 * alignment gcc 4.5 uses for a struct
 */
/* The actual configuration determine if the init/exit sections
 * are handled as text/data or they can be discarded (which
 * often happens at runtime)
 */
/*
 * .data section
 */
/*
 * Data section helpers
 */
/*
 * Allow architectures to handle ro_after_init data on their
 * own by defining an empty RO_AFTER_INIT_DATA.
 */
/*
 * Read only Data
 */
/* RODATA & RO_DATA provided for backward compatibility.
 * All archs are supposed to use RO_DATA() */
/*
 * .text section. Map to function alignment to avoid address changes
 * during second ld run in second ld pass when generating System.map
 *
 * TEXT_MAIN here will match .text.fixup and .text.unlikely if dead
 * code elimination is enabled, so these sections should be converted
 * to use ".." first.
 */
/* sched.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* spinlock.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* Section used for early init (in .S files) */
/*
 * Exception table
 */
/*
 * Init task
 */
/* init and exit section handling */
/*
 * bss (Block Started by Symbol) - uninitialized data
 * zeroed during startup
 */
/*
 * Allow archectures to redefine BSS_FIRST_SECTIONS to add extra
 * sections to the front of bss.
 */
/*
 * DWARF debug sections.
 * Symbols in the DWARF debugging sections are relative to
 * the beginning of the section so we begin them at 0.
 */
  /* Stabs debugging sections.  */
/*
 * Default discarded sections.
 *
 * Some archs want to discard exit text/data at runtime rather than
 * link time due to cross-section references such as alt instructions,
 * bug table, eh_frame, etc.  DISCARDS must be the last of output
 * section definitions so that such archs put those in earlier section
 * definitions.
 */
/**
 * PERCPU_INPUT - the percpu input sections
 * @cacheline: cacheline size
 *
 * The core percpu section names and core symbols which do not rely
 * directly upon load addresses.
 *
 * @cacheline is used to align subsections to avoid false cacheline
 * sharing between subsections for different purposes.
 */
/**
 * PERCPU_VADDR - define output section for percpu area
 * @cacheline: cacheline size
 * @vaddr: explicit base address (optional)
 * @phdr: destination PHDR (optional)
 *
 * Macro which expands to output section for percpu area.
 *
 * @cacheline is used to align subsections to avoid false cacheline
 * sharing between subsections for different purposes.
 *
 * If @vaddr is not blank, it specifies explicit base address and all
 * percpu symbols will be offset from the given address.  If blank,
 * @vaddr always equals @laddr + LOAD_OFFSET.
 *
 * @phdr defines the output PHDR to use if not blank.  Be warned that
 * output PHDR is sticky.  If @phdr is specified, the next output
 * section in the linker script will go there too.  @phdr should have
 * a leading colon.
 *
 * Note that this macros defines __per_cpu_load as an absolute symbol.
 * If there is no need to put the percpu section at a predetermined
 * address, use PERCPU_SECTION.
 */
/**
 * PERCPU_SECTION - define output section for percpu area, simple version
 * @cacheline: cacheline size
 *
 * Align to PAGE_SIZE and outputs output section for percpu area.  This
 * macro doesn't manipulate @vaddr or @phdr and __per_cpu_load and
 * __per_cpu_start will be identical.
 *
 * This macro is equivalent to ALIGN(PAGE_SIZE); PERCPU_VADDR(@cacheline,,)
 * except that __per_cpu_load is defined as a relative symbol against
 * .data..percpu which is required for relocatable x86_32 configuration.
 */
/*
 * Definition of the high level *_SECTION macros
 * They will fit only a subset of the architectures
 */
/*
 * Writeable data.
 * All sections are combined in a single .data section.
 * The sections following CONSTRUCTORS are arranged so their
 * typical alignment matches.
 * A cacheline is typical/always less than a PAGE_SIZE so
 * the sections that has this restriction (or similar)
 * is located before the ones requiring PAGE_SIZE alignment.
 * NOSAVE_DATA starts and ends with a PAGE_SIZE alignment which
 * matches the requirement of PAGE_ALIGNED_DATA.
 *
 * use 0 as page_align if page_aligned data is not used */
/*
 *  arch/arm/include/asm/cache.h
 */
/*
 * Memory returned by kmalloc() may be used for DMA, so we must make
 * sure that all such allocations are cache aligned. Otherwise,
 * unrelated code may cause parts of the buffer to be read into the
 * cache before the transfer is done, causing old data to be seen by
 * the CPU.
 */
/*
 * With EABI on ARMv5 and above we must have 64-bit aligned slab pointers.
 */
/*
 *  arch/arm/include/asm/thread_info.h
 *
 *  Copyright (C) 2002 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Allow us to mark functions as 'deprecated' and have gcc emit a nice
 * warning for each use, in hopes of speeding the functions removal.
 * Usage is:
 * 		int __deprecated foo(void)
 */
/*
 * Allow us to avoid 'defined but not used' warnings on functions and data,
 * as well as force them to be emitted to the assembly file.
 *
 * As of gcc 3.4, static functions that are not marked with attribute((used))
 * may be elided from the assembly file.  As of gcc 3.4, static data not so
 * marked will not be elided, but this may change in a future gcc version.
 *
 * NOTE: Because distributions shipped with a backported unit-at-a-time
 * compiler in gcc 3.3, we must define __used to be __attribute__((used))
 * for gcc >=3.3 instead of 3.4.
 *
 * In prior versions of gcc, such functions and data would be emitted, but
 * would be warned about except with attribute((unused)).
 *
 * Mark functions that are referenced only in inline assembly as __used so
 * the code is emitted even though it appears to be unreferenced.
 */
/*
 * Rather then using noinline to prevent stack consumption, use
 * noinline_for_stack instead.  For documentation reasons.
 */
/*
 * From the GCC manual:
 *
 * Many functions do not examine any values except their arguments,
 * and have no effects except the return value.  Basically this is
 * just slightly more strict class than the `pure' attribute above,
 * since function is not allowed to read global memory.
 *
 * Note that a function that has pointer arguments and examines the
 * data pointed to must _not_ be declared `const'.  Likewise, a
 * function that calls a non-`const' function usually must not be
 * `const'.  It does not make sense for a `const' function to return
 * `void'.
 */
/*
 * Tell gcc if a function is cold. The compiler will assume any path
 * directly leading to the call is unlikely.
 */
/* Simple shorthand for a section definition */
/*
 * Assume alignment of return value.
 */
/* Are two types/vars the same type (ignoring qualifiers)? */
/* Is this type a native word size -- useful for atomic operations */
/* Compile time object size, -1 for unknown */
/*
 * Sparse complains of variable sized arrays due to the temporary variable in
 * __compiletime_assert. Unfortunately we can't just expand it out to make
 * sparse see a constant array size without breaking compiletime_assert on old
 * versions of GCC (e.g. 4.2.4), so hide the array from sparse altogether.
 */
/**
 * compiletime_assert - break build and emit msg if condition is false
 * @condition: a compile-time constant condition to check
 * @msg:       a message to emit if condition is false
 *
 * In tradition of POSIX assert, this macro will break the build if the
 * supplied condition is *false*, emitting the supplied error message if the
 * compiler has support to do so.
 */
/*
 * Prevent the compiler from merging or refetching accesses.  The compiler
 * is also forbidden from reordering successive instances of ACCESS_ONCE(),
 * but only when the compiler is aware of some particular ordering.  One way
 * to make the compiler aware of ordering is to put the two invocations of
 * ACCESS_ONCE() in different C statements.
 *
 * ACCESS_ONCE will only work on scalar types. For union types, ACCESS_ONCE
 * on a union member will work as long as the size of the member matches the
 * size of the union and the size is smaller than word size.
 *
 * The major use cases of ACCESS_ONCE used to be (1) Mediating communication
 * between process-level code and irq/NMI handlers, all running on the same CPU,
 * and (2) Ensuring that the compiler does not  fold, spindle, or otherwise
 * mutilate accesses that either do not require ordering or that interact
 * with an explicit memory barrier or atomic instruction that provides the
 * required ordering.
 *
 * If possible use READ_ONCE()/WRITE_ONCE() instead.
 */
/**
 * lockless_dereference() - safely load a pointer for later dereference
 * @p: The pointer to load
 *
 * Similar to rcu_dereference(), but for situations where the pointed-to
 * object's lifetime is managed by something other than RCU.  That
 * "something other" might be reference counting or simple immortality.
 *
 * The seemingly unused variable ___typecheck_p validates that @p is
 * indeed a pointer type by using a pointer to typeof(*p) as the type.
 * Taking a pointer to typeof(*p) again is needed in case p is void *.
 */
/* Ignore/forbid kprobes attach on very low level functions marked by this attribute: */
/*
 *  arch/arm/include/asm/fpstate.h
 *
 *  Copyright (C) 1995 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/page.h
 *
 *  Copyright (C) 1995-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/* PAGE_SHIFT determines the page size */
/*
 * thread information flags:
 *  TIF_USEDFPU		- FPU was used by this task this quantum (SMP)
 *  TIF_POLLING_NRFLAG	- true if poll_idle() is polling TIF_NEED_RESCHED
 */
/* Checks for any syscall work in entry-common.S */
/*
 * Change these and you break ASM code in entry-common.S
 */
/*
 *  arch/arm/include/asm/memory.h
 *
 *  Copyright (C) 2000-2002 Russell King
 *  modification for nommu, Hyok S. Choi, 2004
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Note: this file should not be included by non-asm/.h files
 */
/* const.h: Macros for dealing with constants.  */
/* Some constant macros are used in both assembler and
 * C code.  Therefore we cannot annotate them always with
 * 'UL' and other type specifiers unilaterally.  We
 * use the following macros to deal with this.
 *
 * Similarly, _AT() will cast an expression with a type in C, but
 * leave it unchanged in asm.
 */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */
/*
 * There seems to be no way of detecting this automatically from user
 * space, so 64 bit architectures should override this in their
 * bitsperlong.h. In particular, an architecture that supports
 * both 32 and 64 bit user space must not rely on CONFIG_64BIT
 * to decide it, but rather check a compiler provided macro.
 */
/*
 * FIXME: The check currently breaks x86-64 build, so it's
 * temporarily disabled. Please fix x86-64 and reenable
 */
/*
 * The C99 types uintXX_t that are usually defined in 'stdint.h' are not as
 * unambiguous on ARM as you would expect. For the types below, there is a
 * difference on ARM between GCC built for bare metal ARM, GCC built for glibc
 * and the kernel itself, which results in build errors if you try to build with
 * -ffreestanding and include 'stdint.h' (such as when you include 'arm_neon.h'
 * in order to use NEON intrinsics)
 *
 * As the typedefs for these types in 'stdint.h' are based on builtin defines
 * supplied by GCC, we can tweak these to align with the kernel's idea of those
 * types, so 'linux/types.h' and 'stdint.h' can be safely included from the same
 * source file (provided that -ffreestanding is used).
 *
 *                    int32_t         uint32_t               uintptr_t
 * bare metal GCC     long            unsigned long          unsigned int
 * glibc GCC          int             unsigned int           unsigned int
 * kernel             int             unsigned int           unsigned long
 */
/*
 * include/linux/sizes.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Allow for constants defined here to be used from assembly code
 * by prepending the UL suffix only with actual C code compilation.
 */
/* PAGE_OFFSET - the virtual address of the start of the kernel image */
/*
 * TASK_SIZE - the maximum size of a user space task.
 * TASK_UNMAPPED_BASE - the lower boundary of the mmap VM area
 */
/*
 * The maximum size of a 26-bit user space task.
 */
/*
 * The module space lives between the addresses given by TASK_SIZE
 * and PAGE_OFFSET - it must be within 32MB of the kernel text.
 */
/* smaller range for Thumb-2 symbols relocation (2^24)*/
/*
 * The highmem pkmap virtual space shares the end of the module area.
 */
/*
 * The XIP kernel gets mapped at the bottom of the module vm area.
 * Since we use sections to map it, this macro replaces the physical address
 * with its virtual address while keeping offset from the base section.
 */
/*
 * We fix the TCM memories max 32 KiB ITCM resp DTCM at these
 * locations
 */
/*
 * Convert a page to/from a physical address
 */
/*
 * PLAT_PHYS_OFFSET is the offset (from zero) of the start of physical
 * memory.  This is used for XIP and NoMMU kernels, and on platforms that don't
 * have CONFIG_ARM_PATCH_PHYS_VIRT. Assembly code must always use
 * PLAT_PHYS_OFFSET and not PHYS_OFFSET.
 */
/*
 *  arch/arm/include/asm/page.h
 *
 *  Copyright (C) 1995-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/pgtable.h
 *
 *  Copyright (C) 1995-2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/* const.h: Macros for dealing with constants.  */
/*
 *  arch/arm/include/asm/proc-fns.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/glue-proc.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/glue.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *  Copyright (C) 2000-2002 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file provides the glue to stick the processor-specific bits
 *  into the kernel in an efficient manner.  The idea is to use branches
 *  when we're only targeting one class of TLB, or indirect calls
 *  when we're targeting multiple classes of TLBs.
 */
/*
 * Work out if we need multiple CPU support
 */
/*
 * CPU_NAME - the prefix for CPU related functions
 */
/*
 * Cortex-A9 needs a different suspend/resume function, so we need
 * multiple CPU support for ARMv7 anyway.
 */
/*
 *  arch/arm/include/asm/page.h
 *
 *  Copyright (C) 1995-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/memory.h
 *
 *  Copyright (C) 2000-2002 Russell King
 *  modification for nommu, Hyok S. Choi, 2004
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Note: this file should not be included by non-asm/.h files
 */
/*
 *  arch/arm/include/asm/pgtable-hwdef.h
 *
 *  Copyright (C) 1995-2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/pgtable-2level-hwdef.h
 *
 *  Copyright (C) 1995-2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Hardware page table definitions.
 *
 * + Level 1 descriptor (PMD)
 *   - common
 */
/*
 *   - section
 */
/*
 *   - coarse table (not used)
 */
/*
 * + Level 2 descriptor (PTE)
 *   - common
 */
/*
 *   - extended small page/tiny page
 */
/*
 *   - small page
 */
/*
 *  arch/arm/include/asm/tlbflush.h
 *
 *  Copyright (C) 1999-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *  arch/arm/include/asm/glue.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *  Copyright (C) 2000-2002 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file provides the glue to stick the processor-specific bits
 *  into the kernel in an efficient manner.  The idea is to use branches
 *  when we're only targeting one class of TLB, or indirect calls
 *  when we're targeting multiple classes of TLBs.
 */
/* Unified Inner Shareable TLB operations (ARMv7 MP extensions) */
/*
 *	MMU TLB Model
 *	=============
 *
 *	We have the following to choose from:
 *	  v4    - ARMv4 without write buffer
 *	  v4wb  - ARMv4 with write buffer without I TLB flush entry instruction
 *	  v4wbi - ARMv4 with write buffer with I TLB flush entry instruction
 *	  fr    - Feroceon (v4wbi with non-outer-cacheable page table walks)
 *	  fa    - Faraday (v4 with write buffer with UTLB)
 *	  v6wbi - ARMv6 with write buffer with I TLB flush entry instruction
 *	  v7wbi - identical to v6wbi
 */
/*
 *  arch/arm/include/asm/pgtable-2level.h
 *
 *  Copyright (C) 1995-2002 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 * Hardware-wise, we have a two level page table structure, where the first
 * level has 4096 entries, and the second level has 256 entries.  Each entry
 * is one 32-bit word.  Most of the bits in the second level entry are used
 * by hardware, and there aren't any "accessed" and "dirty" bits.
 *
 * Linux on the other hand has a three level page table structure, which can
 * be wrapped to fit a two level page table structure easily - using the PGD
 * and PTE only.  However, Linux also expects one "PTE" table per page, and
 * at least a "dirty" bit.
 *
 * Therefore, we tweak the implementation slightly - we tell Linux that we
 * have 2048 entries in the first level, each of which is 8 bytes (iow, two
 * hardware pointers to the second level.)  The second level contains two
 * hardware PTE tables arranged contiguously, preceded by Linux versions
 * which contain the state information Linux needs.  We, therefore, end up
 * with 512 entries in the "PTE" level.
 *
 * This leads to the page tables having the following layout:
 *
 *    pgd             pte
 * |        |
 * +--------+
 * |        |       +------------+ +0
 * +- - - - +       | Linux pt 0 |
 * |        |       +------------+ +1024
 * +--------+ +0    | Linux pt 1 |
 * |        |-----> +------------+ +2048
 * +- - - - + +4    |  h/w pt 0  |
 * |        |-----> +------------+ +3072
 * +--------+ +8    |  h/w pt 1  |
 * |        |       +------------+ +4096
 *
 * See L_PTE_xxx below for definitions of bits in the "Linux pt", and
 * PTE_xxx for definitions of bits appearing in the "h/w pt".
 *
 * PMD_xxx definitions refer to bits in the first level page table.
 *
 * The "dirty" bit is emulated by only granting hardware write permission
 * iff the page is marked "writable" and "dirty" in the Linux PTE.  This
 * means that a write to a clean page will cause a permission fault, and
 * the Linux MM layer will mark the page dirty via handle_pte_fault().
 * For the hardware to notice the permission change, the TLB entry must
 * be flushed, and ptep_set_access_flags() does that for us.
 *
 * The "accessed" or "young" bit is emulated by a similar method; we only
 * allow accesses to the page if the "young" bit is set.  Accesses to the
 * page will cause a fault, and handle_pte_fault() will set the young bit
 * for us as long as the page is marked present in the corresponding Linux
 * PTE entry.  Again, ptep_set_access_flags() will ensure that the TLB is
 * up to date.
 *
 * However, when the "young" bit is cleared, we deny access to the page
 * by clearing the hardware PTE.  Currently Linux does not flush the TLB
 * for us in this case, which means the TLB will retain the transation
 * until either the TLB entry is evicted under pressure, or a context
 * switch which changes the user space mapping occurs.
 */
/*
 * PMD_SHIFT determines the size of the area a second-level page table can map
 * PGDIR_SHIFT determines what a third-level page table entry can map
 */
/*
 * section address mask and size definitions.
 */
/*
 * ARMv6 supersection address mask and size definitions.
 */
/*
 * "Linux" PTE definitions.
 *
 * We keep two sets of PTEs - the hardware and the linux version.
 * This allows greater flexibility in the way we map the Linux bits
 * onto the hardware tables, and allows us to have YOUNG and DIRTY
 * bits.
 *
 * The PTE table pointer refers to the hardware entries; the "Linux"
 * entries are stored 1024 bytes below.
 */
/*
 * These are the memory types, defined to be compatible with
 * pre-ARMv6 CPUs cacheable and bufferable bits: n/a,n/a,C,B
 * ARMv6+ without TEX remapping, they are a table index.
 * ARMv6+ with TEX remapping, they correspond to n/a,TEX(0),C,B
 *
 * MT type		Pre-ARMv6	ARMv6+ type / cacheable status
 * UNCACHED		Uncached	Strongly ordered
 * BUFFERABLE		Bufferable	Normal memory / non-cacheable
 * WRITETHROUGH		Writethrough	Normal memory / write through
 * WRITEBACK		Writeback	Normal memory / write back, read alloc
 * MINICACHE		Minicache	N/A
 * WRITEALLOC		Writeback	Normal memory / write back, write alloc
 * DEV_SHARED		Uncached	Device memory (shared)
 * DEV_NONSHARED	Uncached	Device memory (non-shared)
 * DEV_WC		Bufferable	Normal memory / non-cacheable
 * DEV_CACHED		Writeback	Normal memory / write back, read alloc
 * VECTORS		Variable	Normal memory / variable
 *
 * All normal memory mappings have the following properties:
 * - reads can be repeated with no side effects
 * - repeated reads return the last value written
 * - reads can fetch additional locations without side effects
 * - writes can be repeated (in certain cases) with no side effects
 * - writes can be merged before accessing the target
 * - unaligned accesses can be supported
 *
 * All device mappings have the following properties:
 * - no access speculation
 * - no repetition (eg, on return from an exception)
 * - number, order and size of accesses are maintained
 * - unaligned accesses are "unpredictable"
 */
/*
 * Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after the
 * physical memory until the kernel virtual memory starts.  That means that
 * any out-of-bounds memory accesses will hopefully be caught.
 * The vmalloc() routines leaves a hole of 4kB between each vmalloced
 * area for the same reason. ;)
 */
/*
 * The table below defines the page protection levels that we insert into our
 * Linux page table version.  These get translated into the best that the
 * architecture can perform.  Note that on most ARM hardware:
 *  1) We cannot do execute protection
 *  2) If we could do execute protection, then read is implied
 *  3) write implies read permissions
 */
OUTPUT_ARCH(arm)
ENTRY(stext)
jiffies = jiffies_64;
SECTIONS
{
 /*
	 * XXX: The linker does not define how output sections are
	 * assigned to input sections when there are multiple statements
	 * matching the same input section name.  There is no documented
	 * order of matching.
	 *
	 * unwind exit sections must be discarded before the rest of the
	 * unwind sections get included.
	 */
 /DISCARD/ : {
  *(.ARM.exidx.exit.text)
  *(.ARM.extab.exit.text)
  *(.ARM.exidx.cpuexit.text)
  *(.ARM.extab.cpuexit.text)
 
 
  *(.exitcall.exit)
  *(.discard)
  *(.discard.*)
 }
 . = 0xC0000000 + 0x00008000;
 .head.text : {
  _text = .;
  *(.head.text)
 }
 .text : { /* Real text segment		*/
  _stext = .; /* Text and read-only data	*/
   . = ALIGN(8); __idmap_text_start = .; *(.idmap.text) __idmap_text_end = .; . = ALIGN((1 << 12)); __hyp_idmap_text_start = .; *(.hyp.idmap.text) __hyp_idmap_text_end = .;
   __exception_text_start = .;
   *(.exception.text)
   __exception_text_end = .;
  
  
   . = ALIGN(8); *(.text.hot .text .text.fixup .text.unlikely) *(.ref.text)
   . = ALIGN(8); __sched_text_start = .; *(.sched.text) __sched_text_end = .;
   . = ALIGN(8); __cpuidle_text_start = .; *(.cpuidle.text) __cpuidle_text_end = .;
   . = ALIGN(8); __lock_text_start = .; *(.spinlock.text) __lock_text_end = .;
   __hyp_text_start = .; *(.hyp.text) __hyp_text_end = .;
   . = ALIGN(8); __kprobes_text_start = .; *(.kprobes.text) __kprobes_text_end = .;
   *(.gnu.warning)
   *(.glue_7)
   *(.glue_7t)
  . = ALIGN(4);
  *(.got) /* Global offset table		*/
  
 }
 _etext = .; /* End of text section */
 . = ALIGN(((1 << 12))); .rodata : AT(ADDR(.rodata) - 0) { __start_rodata = .; *(.rodata) *(.rodata.*) __start_data_ro_after_init = .; *(.data..ro_after_init) __end_data_ro_after_init = .; *(__vermagic) . = ALIGN(8); __start___tracepoints_ptrs = .; *(__tracepoints_ptrs) __stop___tracepoints_ptrs = .; *(__tracepoints_strings) } .rodata1 : AT(ADDR(.rodata1) - 0) { *(.rodata1) } . = ALIGN(8); __bug_table : AT(ADDR(__bug_table) - 0) { __start___bug_table = .; *(__bug_table) __stop___bug_table = .; } .pci_fixup : AT(ADDR(.pci_fixup) - 0) { __start_pci_fixups_early = .; *(.pci_fixup_early) __end_pci_fixups_early = .; __start_pci_fixups_header = .; *(.pci_fixup_header) __end_pci_fixups_header = .; __start_pci_fixups_final = .; *(.pci_fixup_final) __end_pci_fixups_final = .; __start_pci_fixups_enable = .; *(.pci_fixup_enable) __end_pci_fixups_enable = .; __start_pci_fixups_resume = .; *(.pci_fixup_resume) __end_pci_fixups_resume = .; __start_pci_fixups_resume_early = .; *(.pci_fixup_resume_early) __end_pci_fixups_resume_early = .; __start_pci_fixups_suspend = .; *(.pci_fixup_suspend) __end_pci_fixups_suspend = .; __start_pci_fixups_suspend_late = .; *(.pci_fixup_suspend_late) __end_pci_fixups_suspend_late = .; } .builtin_fw : AT(ADDR(.builtin_fw) - 0) { __start_builtin_fw = .; *(.builtin_fw) __end_builtin_fw = .; } __ksymtab : AT(ADDR(__ksymtab) - 0) { __start___ksymtab = .; KEEP(*(SORT(___ksymtab+*))) __stop___ksymtab = .; } __ksymtab_gpl : AT(ADDR(__ksymtab_gpl) - 0) { __start___ksymtab_gpl = .; KEEP(*(SORT(___ksymtab_gpl+*))) __stop___ksymtab_gpl = .; } __ksymtab_unused : AT(ADDR(__ksymtab_unused) - 0) { __start___ksymtab_unused = .; KEEP(*(SORT(___ksymtab_unused+*))) __stop___ksymtab_unused = .; } __ksymtab_unused_gpl : AT(ADDR(__ksymtab_unused_gpl) - 0) { __start___ksymtab_unused_gpl = .; KEEP(*(SORT(___ksymtab_unused_gpl+*))) __stop___ksymtab_unused_gpl = .; } __ksymtab_gpl_future : AT(ADDR(__ksymtab_gpl_future) - 0) { __start___ksymtab_gpl_future = .; KEEP(*(SORT(___ksymtab_gpl_future+*))) __stop___ksymtab_gpl_future = .; } __kcrctab : AT(ADDR(__kcrctab) - 0) { __start___kcrctab = .; KEEP(*(SORT(___kcrctab+*))) __stop___kcrctab = .; } __kcrctab_gpl : AT(ADDR(__kcrctab_gpl) - 0) { __start___kcrctab_gpl = .; KEEP(*(SORT(___kcrctab_gpl+*))) __stop___kcrctab_gpl = .; } __kcrctab_unused : AT(ADDR(__kcrctab_unused) - 0) { __start___kcrctab_unused = .; KEEP(*(SORT(___kcrctab_unused+*))) __stop___kcrctab_unused = .; } __kcrctab_unused_gpl : AT(ADDR(__kcrctab_unused_gpl) - 0) { __start___kcrctab_unused_gpl = .; KEEP(*(SORT(___kcrctab_unused_gpl+*))) __stop___kcrctab_unused_gpl = .; } __kcrctab_gpl_future : AT(ADDR(__kcrctab_gpl_future) - 0) { __start___kcrctab_gpl_future = .; KEEP(*(SORT(___kcrctab_gpl_future+*))) __stop___kcrctab_gpl_future = .; } __ksymtab_strings : AT(ADDR(__ksymtab_strings) - 0) { KEEP(*(__ksymtab_strings)) } __init_rodata : AT(ADDR(__init_rodata) - 0) { *(.ref.rodata) } __param : AT(ADDR(__param) - 0) { __start___param = .; *(__param) __stop___param = .; } __modver : AT(ADDR(__modver) - 0) { __start___modver = .; *(__modver) __stop___modver = .; . = ALIGN(((1 << 12))); __end_rodata = .; } . = ALIGN(((1 << 12)));
 . = ALIGN(4);
 __ex_table : AT(ADDR(__ex_table) - 0) {
  __start___ex_table = .;
  *(__ex_table)
  __stop___ex_table = .;
 }
 /*
	 * Stack unwinding tables
	 */
 . = ALIGN(8);
 .ARM.unwind_idx : {
  __start_unwind_idx = .;
  *(.ARM.exidx*)
  __stop_unwind_idx = .;
 }
 .ARM.unwind_tab : {
  __start_unwind_tab = .;
  *(.ARM.extab*)
  __stop_unwind_tab = .;
 }
 .notes : AT(ADDR(.notes) - 0) { __start_notes = .; *(.note.*) __stop_notes = .; }
 . = ALIGN((1 << 12));
 __init_begin = .;
 /*
	 * The vectors and stubs are relocatable code, and the
	 * only thing that matters is their relative offsets
	 */
 __vectors_start = .;
 .vectors 0xffff0000 : AT(__vectors_start) {
  *(.vectors)
 }
 . = __vectors_start + SIZEOF(.vectors);
 __vectors_end = .;
 __stubs_start = .;
 .stubs ADDR(.vectors) + 0x1000 : AT(__stubs_start) {
  *(.stubs)
 }
 . = __stubs_start + SIZEOF(.stubs);
 __stubs_end = .;
 PROVIDE(vector_fiq_offset = vector_fiq - ADDR(.vectors));
 . = ALIGN(8); .init.text : AT(ADDR(.init.text) - 0) { _sinittext = .; *(.init.text) *(.text.startup) *(.meminit.text) _einittext = .; }
 .exit.text : {
  *(.exit.text) *(.text.exit) *(.memexit.text)
 }
 .init.proc.info : {
  . = ALIGN(4); __proc_info_begin = .; *(.proc.info.init) __proc_info_end = .;
 }
 .init.arch.info : {
  __arch_info_begin = .;
  *(.arch.info.init)
  __arch_info_end = .;
 }
 .init.tagtable : {
  __tagtable_begin = .;
  *(.taglist.init)
  __tagtable_end = .;
 }
 .init.smpalt : {
  __smpalt_begin = .;
  *(.alt.smp.init)
  __smpalt_end = .;
 }
 .init.pv_table : {
  __pv_table_begin = .;
  *(.pv_table)
  __pv_table_end = .;
 }
 .init.data : {
  KEEP(*(SORT(___kentry+*))) *(.init.data) *(.meminit.data) *(.init.rodata) *(.meminit.rodata) . = ALIGN(8); __clk_of_table = .; *(__clk_of_table) *(__clk_of_table_end) . = ALIGN(8); __reservedmem_of_table = .; *(__reservedmem_of_table) *(__reservedmem_of_table_end) . = ALIGN(8); __clksrc_of_table = .; *(__clksrc_of_table) *(__clksrc_of_table_end) . = ALIGN(8); __cpu_method_of_table = .; *(__cpu_method_of_table) *(__cpu_method_of_table_end) . = ALIGN(32); __dtb_start = .; *(.dtb.init.rodata) __dtb_end = .; . = ALIGN(8); __irqchip_of_table = .; *(__irqchip_of_table) *(__irqchip_of_table_end)
  . = ALIGN(16); __setup_start = .; *(.init.setup) __setup_end = .;
  __initcall_start = .; KEEP(*(.initcallearly.init)) __initcall0_start = .; KEEP(*(.initcall0.init)) KEEP(*(.initcall0s.init)) __initcall1_start = .; KEEP(*(.initcall1.init)) KEEP(*(.initcall1s.init)) __initcall2_start = .; KEEP(*(.initcall2.init)) KEEP(*(.initcall2s.init)) __initcall3_start = .; KEEP(*(.initcall3.init)) KEEP(*(.initcall3s.init)) __initcall4_start = .; KEEP(*(.initcall4.init)) KEEP(*(.initcall4s.init)) __initcall5_start = .; KEEP(*(.initcall5.init)) KEEP(*(.initcall5s.init)) __initcallrootfs_start = .; KEEP(*(.initcallrootfs.init)) KEEP(*(.initcallrootfss.init)) __initcall6_start = .; KEEP(*(.initcall6.init)) KEEP(*(.initcall6s.init)) __initcall7_start = .; KEEP(*(.initcall7.init)) KEEP(*(.initcall7s.init)) __initcall_end = .; __deferred_initcall_start = .; *(.deferred_initcall.init) __deferred_initcall_end = .;
  __con_initcall_start = .; KEEP(*(.con_initcall.init)) __con_initcall_end = .;
  __security_initcall_start = .; KEEP(*(.security_initcall.init)) __security_initcall_end = .;
 
 }
 .exit.data : {
  *(.exit.data) *(.fini_array) *(.dtors) *(.memexit.data) *(.memexit.rodata)
 }
 . = ALIGN((1 << 12)); .data..percpu : AT(ADDR(.data..percpu) - 0) { __per_cpu_load = .; __per_cpu_start = .; __per_cpu_user_mapped_start = .; *(.data..percpu..first) . = ALIGN((1 << 6)); *(.data..percpu..user_mapped) *(.data..percpu..user_mapped..shared_aligned) . = ALIGN((1 << 12)); *(.data..percpu..user_mapped..page_aligned) __per_cpu_user_mapped_end = .; . = ALIGN((1 << 12)); *(.data..percpu..page_aligned) . = ALIGN((1 << 6)); *(.data..percpu..read_mostly) . = ALIGN((1 << 6)); *(.data..percpu) *(.data..percpu..shared_aligned) __per_cpu_end = .; }
 . = ALIGN(((1 << 12) << 1));
 __init_end = .;
 __data_loc = .;
 .data : AT(__data_loc) {
  _data = .; /* address in memory */
  _sdata = .;
  /*
		 * first, the init task union, aligned
		 * to an 8192 byte boundary.
		 */
  . = ALIGN(((1 << 12) << 1)); __start_init_task = .; *(.data..init_task) __end_init_task = .;
  . = ALIGN((1 << 12)); __nosave_begin = .; *(.data..nosave) . = ALIGN((1 << 12)); __nosave_end = .;
  . = ALIGN((1 << 6)); *(.data..cacheline_aligned)
  . = ALIGN((1 << 6)); *(.data..read_mostly) . = ALIGN((1 << 6));
  /*
		 * and the usual data section
		 */
  *(.data) *(.ref.data) *(.data..shared_aligned) *(.data.unlikely) . = ALIGN(32); *(__tracepoints) . = ALIGN(8); __start___jump_table = .; *(__jump_table) __stop___jump_table = .; . = ALIGN(8); __start___verbose = .; *(__verbose) __stop___verbose = .;
  CONSTRUCTORS
  _edata = .;
 }
 _edata_loc = __data_loc + SIZEOF(.data);
 . = ALIGN(0); __bss_start = .; . = ALIGN(0); .sbss : AT(ADDR(.sbss) - 0) { *(.sbss) *(.scommon) } . = ALIGN(0); .bss : AT(ADDR(.bss) - 0) { *(.bss..page_aligned) *(.dynbss) *(.bss) *(COMMON) } . = ALIGN(0); __bss_stop = .;
 _end = .;
 .stab 0 : { *(.stab) } .stabstr 0 : { *(.stabstr) } .stab.excl 0 : { *(.stab.excl) } .stab.exclstr 0 : { *(.stab.exclstr) } .stab.index 0 : { *(.stab.index) } .stab.indexstr 0 : { *(.stab.indexstr) } .comment 0 : { *(.comment) }
}
/*
 * These must never be empty
 * If you have to comment these two assert statements out, your
 * binutils is too old (for other reasons as well)
 */
ASSERT((__proc_info_end - __proc_info_begin), "missing CPU support")
ASSERT((__arch_info_end - __arch_info_begin), "no machine record defined")
/*
 * The HYP init code can't be more than a page long,
 * and should not cross a page boundary.
 * The above comment applies as well.
 */
ASSERT(__hyp_idmap_text_end - (__hyp_idmap_text_start & (~((1 << 12) - 1))) <= (1 << 12),
 "HYP init code too big or misaligned")
