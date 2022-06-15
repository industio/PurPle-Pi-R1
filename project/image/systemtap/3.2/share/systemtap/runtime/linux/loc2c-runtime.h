/* target operations in the Linux kernel mode
 * Copyright (C) 2005-2017 Red Hat Inc.
 * Copyright (C) 2005-2007 Intel Corporation.
 * Copyright (C) 2007 Quentin Barnes.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _LINUX_LOC2C_RUNTIME_H_
#define _LINUX_LOC2C_RUNTIME_H_

#ifdef STAPCONF_LINUX_UACCESS_H
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif
#include <linux/types.h>
#define intptr_t long
#define uintptr_t unsigned long

#include "../loc2c-runtime.h"


#ifndef STAPCONF_PAGEFAULT_DISABLE  /* before linux commit a866374a */
#define pagefault_disable() preempt_disable()
#define pagefault_enable() preempt_enable_no_resched()
#endif


#define k_fetch_register(regno) \
  pt_regs_fetch_register(c->kregs, regno)
#define k_store_register(regno, value) \
  pt_regs_store_register(c->kregs, regno, value)


/* PR 10601: user-space (user_regset) register access.
   Needs arch specific code, only i386 and x86_64 for now.  */
#if ((defined(STAPCONF_REGSET) || defined(STAPCONF_UTRACE_REGSET)) \
     && (defined (__i386__) || defined (__x86_64__)))

#if defined(STAPCONF_REGSET)
#include <linux/regset.h>
#endif

#if defined(STAPCONF_UTRACE_REGSET)
#include <linux/tracehook.h>
/* adapt new names to old decls */
#define user_regset_view utrace_regset_view
#define user_regset utrace_regset
#define task_user_regset_view utrace_native_view

#else // PR13489, inodes-uprobes export kludge
#if !defined(STAPCONF_TASK_USER_REGSET_VIEW_EXPORTED)
// First typedef from the original decl, then #define it as a typecasted call.
// NB: not all archs actually have the function, but the decl is universal in regset.h
typedef typeof(&task_user_regset_view) task_user_regset_view_fn;
/* Special macro to tolerate the kallsyms function pointer being zero. */
#define task_user_regset_view(t) (kallsyms_task_user_regset_view ? \
                                  (* (task_user_regset_view_fn)(kallsyms_task_user_regset_view))((t)) : \
                                  NULL)
#endif
#endif

struct usr_regset_lut {
  char *name;
  unsigned rsn;
  unsigned pos;
};


/* DWARF register number -to- user_regset bank/offset mapping table.
   The register numbers come from the processor-specific ELF documents.
   The user-regset bank/offset values come from kernel $ARCH/include/asm/user*.h
   or $ARCH/kernel/ptrace.c. */
#if defined (__i386__) || defined (__x86_64__)
static const struct usr_regset_lut url_i386[] = {
  { "ax", NT_PRSTATUS, 6*4 },
  { "cx", NT_PRSTATUS, 1*4 },
  { "dx", NT_PRSTATUS, 2*4 },
  { "bx", NT_PRSTATUS, 0*4 },
  { "sp", NT_PRSTATUS, 15*4 },
  { "bp", NT_PRSTATUS, 5*4 },
  { "si", NT_PRSTATUS, 3*4 },
  { "di", NT_PRSTATUS, 4*4 },
  { "ip", NT_PRSTATUS, 12*4 },
};
#endif

#if defined (__x86_64__)
static const struct usr_regset_lut url_x86_64[] = {
  { "rax", NT_PRSTATUS, 10*8 },
  { "rdx", NT_PRSTATUS, 12*8 },
  { "rcx", NT_PRSTATUS, 11*8 },
  { "rbx", NT_PRSTATUS, 5*8 },
  { "rsi", NT_PRSTATUS, 13*8 },
  { "rdi", NT_PRSTATUS, 14*8 },
  { "rbp", NT_PRSTATUS, 4*8 },
  { "rsp", NT_PRSTATUS, 19*8 },
  { "r8", NT_PRSTATUS, 9*8 },
  { "r9", NT_PRSTATUS, 8*8 }, 
  { "r10", NT_PRSTATUS, 7*8 },  
  { "r11", NT_PRSTATUS, 6*8 }, 
  { "r12", NT_PRSTATUS, 3*8 }, 
  { "r13", NT_PRSTATUS, 2*8 }, 
  { "r14", NT_PRSTATUS, 1*8 }, 
  { "r15", NT_PRSTATUS, 0*8 }, 
  { "rip", NT_PRSTATUS, 16*8 }, 
  /* XXX: SSE registers %xmm0-%xmm7 */ 
  /* XXX: SSE2 registers %xmm8-%xmm15 */
  /* XXX: FP registers %st0-%st7 */
  /* XXX: MMX registers %mm0-%mm7 */
};
#endif
/* XXX: insert other architectures here. */


static u32 ursl_fetch32 (const struct usr_regset_lut* lut, unsigned lutsize, int e_machine, unsigned regno)
{
  u32 value = ~0;
  const struct user_regset_view *rsv = task_user_regset_view(current); 
  unsigned rsi;
  int rc;
  unsigned rsn;
  unsigned pos;
  unsigned count;

  WARN_ON (!rsv);
  if (!rsv) goto out;
  WARN_ON (regno >= lutsize);
  if (regno >= lutsize) goto out;
  if (rsv->e_machine != e_machine) goto out;

  rsn = lut[regno].rsn;
  pos = lut[regno].pos;
  count = sizeof(value);

  for (rsi=0; rsi<rsv->n; rsi++)
    if (rsv->regsets[rsi].core_note_type == rsn)
      {
        const struct user_regset *rs = & rsv->regsets[rsi];
        rc = (rs->get)(current, rs, pos, count, & value, NULL);
        WARN_ON (rc);
        /* success */
        goto out;
      }
  WARN_ON (1); /* did not find appropriate regset! */
  
 out:
  return value;
}


static void ursl_store32 (const struct usr_regset_lut* lut,unsigned lutsize,  int e_machine, unsigned regno, u32 value)
{
  const struct user_regset_view *rsv = task_user_regset_view(current); 
  unsigned rsi;
  int rc;
  unsigned rsn;
  unsigned pos;
  unsigned count;

  WARN_ON (!rsv);
  if (!rsv) goto out;
  WARN_ON (regno >= lutsize);
  if (regno >= lutsize) goto out;
  if (rsv->e_machine != e_machine) goto out;

  rsn = lut[regno].rsn;
  pos = lut[regno].pos;
  count = sizeof(value);

  for (rsi=0; rsi<rsv->n; rsi++)
    if (rsv->regsets[rsi].core_note_type == rsn)
      {
        const struct user_regset *rs = & rsv->regsets[rsi];
        rc = (rs->set)(current, rs, pos, count, & value, NULL);
        WARN_ON (rc);
        /* success */
        goto out;
      }
  WARN_ON (1); /* did not find appropriate regset! */
  
 out:
  return;
}


static u64 ursl_fetch64 (const struct usr_regset_lut* lut, unsigned lutsize, int e_machine, unsigned regno)
{
  u64 value = ~0;
  const struct user_regset_view *rsv = task_user_regset_view(current); 
  unsigned rsi;
  int rc;
  unsigned rsn;
  unsigned pos;
  unsigned count;

  if (!rsv) goto out;
  if (regno >= lutsize) goto out;
  if (rsv->e_machine != e_machine) goto out;

  rsn = lut[regno].rsn;
  pos = lut[regno].pos;
  count = sizeof(value);

  for (rsi=0; rsi<rsv->n; rsi++)
    if (rsv->regsets[rsi].core_note_type == rsn)
      {
        const struct user_regset *rs = & rsv->regsets[rsi];
        rc = (rs->get)(current, rs, pos, count, & value, NULL);
        if (rc)
          goto out;
        /* success */
        return value;
      }
 out:
  printk (KERN_WARNING "process %d mach %d regno %d not available for fetch.\n", current->tgid, e_machine, regno);
  return value;
}


static void ursl_store64 (const struct usr_regset_lut* lut,unsigned lutsize,  int e_machine, unsigned regno, u64 value)
{
  const struct user_regset_view *rsv = task_user_regset_view(current); 
  unsigned rsi;
  int rc;
  unsigned rsn;
  unsigned pos;
  unsigned count;

  WARN_ON (!rsv);
  if (!rsv) goto out;
  WARN_ON (regno >= lutsize);
  if (regno >= lutsize) goto out;
  if (rsv->e_machine != e_machine) goto out;

  rsn = lut[regno].rsn;
  pos = lut[regno].pos;
  count = sizeof(value);

  for (rsi=0; rsi<rsv->n; rsi++)
    if (rsv->regsets[rsi].core_note_type == rsn)
      {
        const struct user_regset *rs = & rsv->regsets[rsi];
        rc = (rs->set)(current, rs, pos, count, & value, NULL);
        if (rc)
          goto out;
        /* success */
        return;
      }
  
 out:
  printk (KERN_WARNING "process %d mach %d regno %d not available for store.\n", current->tgid, e_machine, regno);
  return;
}


#if defined (__i386__)

#define u_fetch_register(regno) ursl_fetch32(url_i386, ARRAY_SIZE(url_i386), EM_386, regno)
#define u_store_register(regno,value) ursl_store32(url_i386, ARRAY_SIZE(url_i386), EM_386, regno, value)

#elif defined (__x86_64__)

#define u_fetch_register(regno) (_stp_is_compat_task() ? ursl_fetch32(url_i386, ARRAY_SIZE(url_i386), EM_386, regno) : ursl_fetch64(url_x86_64, ARRAY_SIZE(url_x86_64), EM_X86_64, regno))
#define u_store_register(regno,value)  (_stp_is_compat_task() ? ursl_store32(url_i386, ARRAY_SIZE(url_i386), EM_386, regno, value) : ursl_store64(url_x86_64, ARRAY_SIZE(url_x86_64), EM_X86_64, regno, value))

#endif

#else /* ! STAPCONF_REGSET */

/* Downgrade to pt_dwarf_register access. */

#define u_store_register(regno, value) \
  pt_regs_store_register(c->uregs, regno, value)

/* If we're in a 32/31-bit task in a 64-bit kernel, we need to emulate
 * 32-bitness by masking the output of pt_regs_fetch_register() */
#ifndef CONFIG_COMPAT
#define u_fetch_register(regno) \
  pt_regs_fetch_register(c->uregs, regno)
#else
#define u_fetch_register(regno) \
  (_stp_is_compat_task() ? (0xffffffff & pt_regs_fetch_register(c->uregs, regno)) \
                         : pt_regs_fetch_register(c->uregs, regno))
#endif

#endif /* STAPCONF_REGSET */


/* The deref and store_deref macros are called to safely access
   addresses in the probe context.  These macros are used only for
   kernel addresses.  The macros must handle bogus addresses here
   gracefully (as from corrupted data structures, stale pointers,
   etc), by doing a "goto deref_fault".

   On most machines, the asm/uaccess.h macros __get_user and
   __put_user macros do exactly the low-level work we need to access
   memory with fault handling, and are not actually specific to
   user-address access at all.  */

/*
 * On most platforms, __get_user_inatomic() and __put_user_inatomic()
 * are defined, which are the same as __get_user() and __put_user(),
 * but without a call to might_sleep(). Since we're disabling page
 * faults when we read, we want to use the 'inatomic' variants when
 * available.
 */
#ifdef __get_user_inatomic
#define __stp_get_user __get_user_inatomic
#else
#define __stp_get_user __get_user
#endif
#ifdef __put_user_inatomic
#define __stp_put_user __put_user_inatomic
#else
#define __stp_put_user __put_user
#endif

/*
 * Some arches (like aarch64) don't have __get_user_bad() or
 * __put_user_bad(), so use BUILD_BUG() instead.
 */
#ifdef BUILD_BUG
#define __stp_get_user_bad BUILD_BUG
#define __stp_put_user_bad BUILD_BUG
#else
#define __stp_get_user_bad __get_user_bad
#define __stp_put_user_bad __put_user_bad
#endif


/* 
 * __stp_deref_nocheck(): reads a simple type from a
 * location with no address sanity checking.
 *
 * value: read the simple type into this variable
 * size: number of bytes to read
 * addr: address to read from
 *
 * Note that the caller *must* check the address for validity and do
 * any other checks necessary. This macro is designed to be used as
 * the base for the other macros more suitable for the rest of the
 * code to use. Note that the caller is also responsible for ensuring
 * that the kernel doesn't pagefault while reading.
 */

static inline int __stp_deref_nocheck_(u64 *pv, size_t size, void *addr)
{
  u64 v = 0;
  int r = -EFAULT;

  switch (size)
    {
    case 1: { u8 b; r = __stp_get_user(b, (u8 *)addr); v = b; } break;
    case 2: { u16 w; r = __stp_get_user(w, (u16 *)addr); v = w; } break;
    case 4: { u32 l; r = __stp_get_user(l, (u32 *)addr); v = l; } break;
#if defined(__i386__) || defined(__arm__)
    /* x86 and arm can't do 8-byte get_user, so we have to split it */
    case 8: { union { u32 l[2]; u64 ll; } val;
	      r = __stp_get_user(val.l[0], &((u32 *)addr)[0]);
	      if (! r)
		  r = __stp_get_user(val.l[1], &((u32 *)addr)[1]);
	      if (! r)
		  v = val.ll;
	    } break;
#else
    case 8: { r = __stp_get_user(v, (u64 *)addr); } break;
#endif
    }

  *pv = v;
  return r;
}

#define __stp_deref_nocheck(value, size, addr)				\
  ({									\
    u64 _v = 0; int _e = -EFAULT;					\
    switch (size)							\
      {									\
      case 1: case 2: case 4: case 8:					\
	_e = __stp_deref_nocheck_(&_v, (size), (void *)(uintptr_t)(addr)); \
	(value) = _v;							\
	break;								\
      default:								\
	__stp_get_user_bad();						\
      }									\
    _e;									\
  })


/* 
 * _stp_lookup_bad_addr(): safely verify an address
 *
 * type: memory access type (either VERIFY_READ or VERIFY_WRITE)
 * size: number of bytes to verify
 * addr: address to verify
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * The macro returns 0 if the address is valid, non-zero otherwise.
 * Note that the kernel will not pagefault when trying to verify the
 * memory. Also note that no DEREF_FAULT will occur when verifying the
 * memory.
 */

static inline int _stp_lookup_bad_addr_(int type, size_t size,
                                        uintptr_t addr, mm_segment_t seg)
{
  int bad;
  mm_segment_t oldfs = get_fs();

  set_fs(seg);
  pagefault_disable();
  bad = lookup_bad_addr(type, addr, size);
  pagefault_enable();
  set_fs(oldfs);

  return bad;
}

#define _stp_lookup_bad_addr(type, size, addr, seg) \
  _stp_lookup_bad_addr_((type), (size), (uintptr_t)(addr), (seg))


/* 
 * _stp_deref_nofault(): safely read a simple type from memory without
 * a DEREF_FAULT on error
 *
 * value: read the simple type into this variable
 * size: number of bytes to read
 * addr: address to read from
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * If this macro gets an error while trying to read memory, nonzero is
 * returned. On success, 0 is return. Note that the kernel will not
 * pagefault when trying to read the memory.
 */

static inline int _stp_deref_nofault_(u64 *pv, size_t size, void *addr,
				      mm_segment_t seg)
{
  int r = -EFAULT;
  mm_segment_t oldfs = get_fs();

  set_fs(seg);
  pagefault_disable();
  if (lookup_bad_addr(VERIFY_READ, (uintptr_t)addr, size))
    r = -EFAULT;
  else
    r = __stp_deref_nocheck_(pv, size, addr);
  pagefault_enable();
  set_fs(oldfs);

  return r;
}

#define _stp_deref_nofault(value, size, addr, seg)			\
  ({									\
    u64 _v = 0; int _e = -EFAULT;					\
    switch (size)							\
      {									\
      case 1: case 2: case 4: case 8:					\
	_e = _stp_deref_nofault_					\
		(&_v, (size), (void *)(uintptr_t)(addr), (seg));	\
	break;								\
      default:								\
	__stp_get_user_bad();						\
      }									\
    (value) = _v;							\
    _e;									\
  })


/* 
 * _stp_deref(): safely read a simple type from memory
 *
 * size: number of bytes to read
 * addr: address to read from
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * The macro returns the value read. If this macro gets an error while
 * trying to read memory, a DEREF_FAULT will occur. Note that the
 * kernel will not pagefault when trying to read the memory.
 */

#define _stp_deref(size, addr, seg)					\
  ({									\
    u64 _v = 0; int _e = -EFAULT;					\
    switch (size)							\
      {									\
      case 1: case 2: case 4: case 8:					\
	_e = _stp_deref_nofault_					\
		(&_v, (size), (void *)(uintptr_t)(addr), (seg));	\
	break;								\
      default:								\
	__stp_get_user_bad();						\
      }									\
    if (_e)								\
      DEREF_FAULT(addr);						\
    _v;									\
  })


/* 
 * __stp_store_deref_nocheck(): writes a simple type to a location
 * with no address sanity checking.
 *
 * size: number of bytes to write
 * addr: address to write to
 * value: read the simple type from this variable
 *
 * Note that the caller *must* check the address for validity and do
 * any other checks necessary. This macro is designed to be used as
 * the base for the other macros more suitable for the rest of the
 * code to use. Note that the caller is also responsible for ensuring
 * that the kernel doesn't pagefault while writing.
 */

static inline int __stp_store_deref_nocheck_(size_t size, void *addr, u64 v)
{
  int r;
  switch (size)
    {
    case 1: r = __stp_put_user((u8)v, (u8 *)addr); break;
    case 2: r = __stp_put_user((u16)v, (u16 *)addr); break;
    case 4: r = __stp_put_user((u32)v, (u32 *)addr); break;
#if defined(__i386__) || defined(__arm__)
    /* x86 and arm can't do 8-byte put_user, so we have to split it */
    default: { union { u32 l[2]; u64 ll; } val;
	       val.ll = v;
	       r = __stp_put_user(val.l[0], &((u32 *)addr)[0]);
	       if (! r)
		   r = __stp_put_user(val.l[1], &((u32 *)addr)[1]);
	     } break;
#else
    default: r = __stp_put_user(v, (u64 *)addr); break;
#endif
    }
  return r;
}

#define __stp_store_deref_nocheck(size, addr, value)			\
  ({									\
    int _e = -EFAULT;							\
    switch (size)							\
      {									\
      case 1: case 2: case 4: case 8:					\
	_e = __stp_store_deref_nocheck_					\
		((size), (void *)(uintptr_t)(addr), (value));		\
	break;								\
      default:								\
        __stp_put_user_bad();						\
      }									\
    _e;									\
  })


/* 
 * _stp_store_deref(): safely write a simple type to memory
 *
 * size: number of bytes to write
 * addr: address to write to
 * value: read the simple type from this variable
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * The macro has no return value. If this macro gets an error while
 * trying to write, a STORE_DEREF_FAULT will occur. Note that the
 * kernel will not pagefault when trying to write the memory.
 */

static inline int _stp_store_deref_(size_t size, void *addr, u64 v,
				    mm_segment_t seg)
{
  int r;
  mm_segment_t oldfs = get_fs();

  set_fs(seg);
  pagefault_disable();
  if (lookup_bad_addr(VERIFY_READ, (uintptr_t)addr, size))
    r = -EFAULT;
  else
    r = __stp_store_deref_nocheck_(size, addr, v);
  pagefault_enable();
  set_fs(oldfs);

  return r;
}

#define _stp_store_deref(size, addr, value, seg)			\
  ({									\
    int _e = -EFAULT;							\
    switch (size)							\
      {									\
      case 1: case 2: case 4: case 8:					\
	_e = _stp_store_deref_						\
		((size), (void *)(uintptr_t)(addr), (value), (seg));	\
	break;								\
      default:								\
        __stp_put_user_bad();						\
      }									\
    if (_e)								\
      STORE_DEREF_FAULT(addr);						\
    (void)0;								\
  })


/* Map kderef/uderef to the generic segment-aware deref macros. */ 

#ifndef kderef
#define kderef(s,a) _stp_deref(s,a,KERNEL_DS)
#endif

#ifndef store_kderef
#define store_kderef(s,a,v) _stp_store_deref(s,a,v,KERNEL_DS)
#endif

#ifndef uderef
#define uderef(s,a) _stp_deref(s,a,USER_DS)
#endif

#ifndef store_uderef
#define store_uderef(s,a,v) _stp_store_deref(s,a,v,USER_DS)
#endif

#ifndef CONFIG_64BIT

/* The kderef/uderef macros (which is what Xderef gets set to), alway
 * returns a 64-bit value. This causes a problem on a 32-bit system
 * when we want to cast the 64-bit value to a 32-bit pointer - gcc
 * gives a "cast to pointer from integer of different size" error. So,
 * we'll cast it to a u32 before doing the final cast to the actual
 * type. */

#define __Xread(ptr, Xderef)						\
  ((sizeof(*(ptr)) == 8)						\
   ? *(typeof(ptr))&(u64) { Xderef(sizeof(*(ptr)), (ptr)) }		\
   : *(typeof(ptr))&(u32) { (u32) Xderef(sizeof(*(ptr)), (ptr)) } )

/* For __Xwrite, we need to handle the case where 'value' is a pointer
 * and avoid the "cast from pointer to integer of different size" gcc
 * errors. */

#define __Xwrite(ptr, value, store_Xderef)				  \
  ({									  \
    if (sizeof(*(ptr)) == 8) {						  \
      union { typeof(*(ptr)) v; u64 l; } _kw;				  \
      _kw.v = (typeof(*(ptr)))(value);					  \
      store_Xderef(8, (ptr), _kw.l);					  \
    } else								  \
      store_Xderef(sizeof(*(ptr)), (ptr), (long)(typeof(*(ptr)))(value)); \
  })

#else

#define __Xread(ptr, Xderef) \
  ( (typeof(*(ptr))) Xderef(sizeof(*(ptr)), (ptr)) )
#define __Xwrite(ptr, value, store_Xderef) \
  ( store_Xderef(sizeof(*(ptr)), (ptr), (long)(typeof(*(ptr)))(value)) )

#endif

#define kread(ptr) __Xread((ptr), kderef)
#define uread(ptr) __Xread((ptr), uderef)

#define kwrite(ptr, value) __Xwrite((ptr), (value), store_kderef)
#define uwrite(ptr, value) __Xwrite((ptr), (value), store_uderef)


/* Dereference a kernel buffer ADDR of size MAXBYTES. Put the bytes in
 * address DST (which can be NULL).
 *
 * This function is useful for reading memory when the size isn't a
 * size that kderef() handles.  This function is very similar to
 * kderef_string(), but kderef_buffer() doesn't quit when finding a
 * '\0' byte or append a '\0' byte.
 */

static inline char *kderef_buffer_(char *dst, void *addr, size_t len)
{
  int err = 0;
  size_t i;
  mm_segment_t oldfs = get_fs();

  set_fs(KERNEL_DS);
  pagefault_disable();
  if (lookup_bad_addr(VERIFY_READ, (uintptr_t)addr, len))
    err = 1;
  else
    for (i = 0; i < len; ++i)
      {
	u8 v;
	err = __stp_get_user(v, (u8 *)addr + i);
	if (err)
	  break;
	if (dst)
	  *dst++ = v;
      }
  pagefault_enable();
  set_fs(oldfs);

  return err ? (char *)-1 : dst;
}

#define kderef_buffer(dst, addr, maxbytes)				\
  ({									\
    char *_r = kderef_buffer_((dst), (void *)(uintptr_t)(addr), (maxbytes)); \
    if (_r == (char *)-1)						\
      DEREF_FAULT(addr);						\
    _r;									\
  })

/* 
 * _stp_deref_string_nofault(): safely read a string from memory
 * without a DEREF_FAULT on error
 *
 * dst: read the string into this address
 * addr: address to read from
 * len: maximum number of bytes to read
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * If this function gets an error while trying to read memory, -EFAULT
 * is returned. On success, the number of bytes copied is returned
 * (not including the trailing NULL). Note that the kernel will not
 * pagefault when trying to read the string.
 */

static inline long _stp_deref_string_nofault(char *dst, const char *addr,
					     size_t len, mm_segment_t seg)
{
  int err = 0;
  size_t i = 0;
  mm_segment_t oldfs = get_fs();

  set_fs(seg);
  pagefault_disable();
  if (lookup_bad_addr(VERIFY_READ, (uintptr_t)addr, len))
    err = 1;
  else
    {
      for (i = 0; i < len; ++i)
	{
	  u8 v;
	  err = __stp_get_user(v, (u8 *)addr + i);
	  if (err || v == '\0')
	    break;
	  if (dst)
	    *dst++ = v;
	}
      if (!err && dst)
	*dst = '\0';
    }
  pagefault_enable();
  set_fs(oldfs);

  return err ? -EFAULT : i;
}

#define kderef_string(dst, addr, maxbytes)				\
  ({									\
    long _r = _stp_deref_string_nofault((dst), (void *)(uintptr_t)(addr), (maxbytes), KERNEL_DS); \
    if (_r < 0)								\
      DEREF_FAULT(addr);						\
    _r;									\
  })

/* 
 * _stp_store_deref_string(): safely write a string to memory
 *
 * src: source string
 * addr: address to write to
 * maxbytes: maximum number of bytes to write
 * seg: memory segment to use, either kernel (KERNEL_DS) or user
 * (USER_DS)
 * 
 * The macro has no return value. If this macro gets an error while
 * trying to write, a STORE_DEREF_FAULT will occur. Note that the
 * kernel will not pagefault when trying to write the memory.
 */

static inline int _stp_store_deref_string_(char *src, void *addr, size_t len,
					   mm_segment_t seg)
{
  int err = 0;
  size_t i;
  mm_segment_t oldfs = get_fs();

  set_fs(seg);
  pagefault_disable();
  if (lookup_bad_addr(VERIFY_WRITE, (uintptr_t)addr, len))
    err = 1;
  else if (len > 0)
    {
      for (i = 0; i < len - 1; ++i)
	{
	  err = __stp_put_user(*src++, (u8 *)addr + i);
	  if (err)
	    goto out;
	}
      err = __stp_put_user('\0', (u8 *)addr + i);
    }
 out:
  pagefault_enable();
  set_fs(oldfs);

  return err;
}

#define _stp_store_deref_string(src, addr, maxbytes, seg)		\
  ({									\
    if (_stp_store_deref_string_					\
	((src), (void *)(uintptr_t)(addr), (maxbytes), (seg)))		\
      STORE_DEREF_FAULT(addr);						\
    (void)0;								\
  })


/* 
 * store_kderef_string(): safely write a string to kernel memory
 *
 * src: source string
 * addr: address to write to
 * maxbytes: maximum number of bytes to write
 * 
 * The macro has no return value. If this macro gets an error while
 * trying to write, a STORE_DEREF_FAULT will occur. Note that the
 * kernel will not pagefault when trying to write the memory.
 */

#define store_kderef_string(src, addr, maxbytes)                              \
  _stp_store_deref_string((src), (addr), (maxbytes), KERNEL_DS)


/* 
 * store_uderef_string(): safely write a string to user memory
 *
 * src: source string
 * addr: address to write to
 * maxbytes: maximum number of bytes to write
 * 
 * The macro has no return value. If this macro gets an error while
 * trying to write, a STORE_DEREF_FAULT will occur. Note that the
 * kernel will not pagefault when trying to write the memory.
 */

#define store_uderef_string(src, addr, maxbytes)                              \
  _stp_store_deref_string((src), (addr), (maxbytes), USER_DS)

#endif /* _LINUX_LOC2C_RUNTIME_H_ */
