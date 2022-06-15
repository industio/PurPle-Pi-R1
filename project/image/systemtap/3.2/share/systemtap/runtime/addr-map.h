/* -*- linux-c -*- 
 * Header file containing declarations for addr-map.c.
 *
 * Copyright (C) 2016 Red Hat Inc.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _ADDR_MAP_H_
#define _ADDR_MAP_H_ 1

#if defined(__KERNEL__)

#include <linux/uaccess.h>

#elif defined(__DYNINST__)

#include <sys/types.h>
#ifndef VERIFY_READ
#define VERIFY_READ	0
#endif
#ifndef VERIFY_WRITE
#define VERIFY_WRITE	1
#endif

#endif

static int lookup_bad_addr(const int type, const unsigned long addr,
			   const size_t size);

#endif /* _ADDR_MAP_H_ */
