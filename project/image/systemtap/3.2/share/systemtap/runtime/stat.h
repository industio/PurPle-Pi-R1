/* -*- linux-c -*- 
 * Statistics Header
 * Copyright (C) 2005-2016 Red Hat Inc.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _STAT_H_
#define _STAT_H_

/* maximum buckets for a linear histogram */
#ifndef STP_MAX_BUCKETS
#define STP_MAX_BUCKETS 128
#endif

/* buckets for log histogram. */
#define HIST_LOG_BUCKETS 128
#define HIST_LOG_BUCKET0 64

/* statistical operations used with a global */
#define STAT_OP_COUNT     1 << 1
#define STAT_OP_SUM       1 << 2
#define STAT_OP_MIN       1 << 3
#define STAT_OP_MAX       1 << 4
#define STAT_OP_AVG       1 << 5
#define STAT_OP_VARIANCE  1 << 6

/** other defines used for passing translator information to the runtime
    values must not collide with the above statistical operations defines */
#define KEY_MAPENTRIES    1 << 7
#define KEY_STAT_WRAP     1 << 8
#define KEY_HIST_TYPE     1 << 9

/** histogram type */
enum histtype { HIST_NONE, HIST_LOG, HIST_LINEAR };

/** Statistics are stored in this struct.  This is per-cpu or per-node data 
    and is variable length due to the unknown size of the histogram. */
struct stat_data {
	int shift;
	int stat_ops;
	int64_t count;
	int64_t sum;
	int64_t min, max;
	int64_t avg_s;
	int64_t _M2;
	int64_t variance;
	int64_t variance_s;
	int64_t histogram[];
};
typedef struct stat_data stat_data;

/** Information about the histogram data collected. This data 
    is global and not duplicated per-cpu. */

struct _Hist {
	enum histtype type;
	int start;
	int stop;
	int interval;
	int buckets;
	int bit_shift;
	int stat_ops;
};
typedef struct _Hist *Hist;

/* The specific runtimes define struct _Stat and its alloc/free */
#if defined(__KERNEL__)
#include "linux/stat_runtime.h"
#elif defined(__DYNINST__)
#include "dyninst/stat_runtime.h"
#endif

#endif /* _STAT_H_ */
