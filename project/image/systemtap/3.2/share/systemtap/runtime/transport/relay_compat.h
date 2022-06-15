/* -*- linux-c -*- */
#ifndef _TRANSPORT_RELAY_COMPAT_H_
#define _TRANSPORT_RELAY_COMPAT_H_

#if defined (CONFIG_RELAYFS_FS) || defined (CONFIG_RELAYFS_FS_MODULE)
#  include <linux/relayfs_fs.h>
#elif defined (CONFIG_RELAY)
#  include <linux/relay.h>
#endif

#ifdef STAPCONF_RELAY_BUF_PER_CPU_PTR

#include <linux/percpu.h>

#define _stp_get_rchan_subbuf(rchan_buf, cpu) \
	(*per_cpu_ptr((rchan_buf), (cpu)))

#else

#define _stp_get_rchan_subbuf(rchan_buf, cpu) \
	((rchan_buf)[(cpu)])

#endif

#endif /* _TRANSPORT_RELAY_COMPAT_H_ */
