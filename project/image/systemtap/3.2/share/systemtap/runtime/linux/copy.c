/* -*- linux-c -*- 
 * Copy from user space functions
 * Copyright (C) 2005-2017 Red Hat Inc.
 * Copyright (C) 2005 Intel Corporation.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _STAPLINUX_COPY_C_		/* -*- linux-c -*- */
#define _STAPLINUX_COPY_C_

/** @file copy.c
 * @brief Functions to copy from user space.
 */

/** @addtogroup copy Functions to copy from user space.
 * Functions to copy from user space.
 * None of these functions will sleep (for example to allow pages
 * to be swapped in). It is possible (although rare) that the data
 * in user space will not present and these functions will return an error.
 * @{
 */

/** Copy a NULL-terminated string from userspace.
 * On success, returns the length of the string (not including the trailing
 * NULL).
 *
 * If access to userspace fails, returns -EFAULT (some data may have been
 * copied).
 * @param dst Destination address, in kernel space.  This buffer must be at
 *         least <i>count</i> bytes long.
 * @param src Source address, in user space.
 * @param count Maximum number of bytes to copy, including the trailing NULL.
 * 
 * If <i>count</i> is smaller than the length of the string, copies 
 * <i>count</i> bytes and returns <i>count</i>.
 */

static long _stp_strncpy_from_user(char *dst, const char __user *src,
				   long count)
{
	return _stp_deref_string_nofault(dst, src, count, USER_DS);
}

/** Copy a block of data from user space.
 *
 * If data could not be copied, this function will not modify the
 * destination.
 *
 * @param dst Destination address, in kernel space.
 * @param src Source address, in user space.
 * @param count Number of bytes to copy.
 * @return number of bytes that could not be copied. On success, 
 * this will be zero.
 *
 */

/* XXX: see also kread/uread in loc2c-runtime.h */
static unsigned long _stp_copy_from_user(char *dst, const char __user *src, unsigned long count)
{
	if (count) {
                mm_segment_t _oldfs = get_fs();
                set_fs(USER_DS);
                pagefault_disable();
		if (!lookup_bad_addr(VERIFY_READ, (const unsigned long)src, count))
			count = __copy_from_user_inatomic(dst, src, count);
		else
			/* Notice that if we fail, we don't modify
			 * the destination. In the failure case, we
			 * can't trust 'count' to be reasonable. */
			count = -EFAULT;
                pagefault_enable();
                set_fs(_oldfs);
	}
	return count;
}

/** @} */
#endif /* _STAPLINUX_COPY_C_ */
