/* -*- linux-c -*-
 *
 * /proc probe definitions
 * Copyright (C) 2016 Red Hat Inc.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _STP_PROCFS_PROBES_H_
#define _STP_PROCFS_PROBES_H_

// Currently we have to output _stp_procfs_data early in the
// translation process.  It really should go here.
struct _stp_procfs_data {
	char *buffer;
	size_t bufsize;
	size_t count;
};

#ifndef STP_PROCFS_BUFSIZE
#define STP_PROCFS_BUFSIZE MAXSTRINGLEN
#endif

#endif	/* _STP_PROCFS_PROBES_H_ */
