//
// The following kernel commit:
//
// commit 017c59c042d01fc84cae7a8ea475861e702c77ab
// Author: Akash Goel <akash.goel@intel.com>
// Date:   Fri Sep 2 21:47:38 2016 +0200
//
//     relay: Use per CPU constructs for the relay channel buffer pointers
//
//     relay essentially needs to maintain a per CPU array of channel buffer
//     pointers but it manually creates that array.  Instead its better to use
//     the per CPU constructs, provided by the kernel, to allocate & access the
//     array of pointer to channel buffers.
//
// changed the way the 'rchan->buf' field works. It just to be a
// regular array, and is now a per_cpu_ptr-style array.

#include <linux/relay.h>
#include <linux/percpu.h>
#include <linux/bug.h>

struct rchan_buf *relay_buf_test(struct rchan *chan, unsigned int cpu)
{
    BUILD_BUG_ON(sizeof(chan->buf) != sizeof(struct rchan_buf **));
    return *per_cpu_ptr(chan->buf, cpu);
}
