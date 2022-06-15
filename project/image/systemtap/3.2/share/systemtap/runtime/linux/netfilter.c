#ifndef __NETFILTER_C__
#define __NETFILTER_C__

#ifndef STAPCONF_NF_REGISTER_HOOK

// The following kernel commit removed nf_register_hook(), which the
// netfilter probes use:
//
//   commit cf56c2f892a8a1870a8358114ad896772da7543a
//   Author: Florian Westphal <fw@strlen.de>
//   Date:   Thu Jul 6 23:17:44 2017 +0200
//   
//       netfilter: remove old pre-netns era hook api
//       
//       no more users in the tree, remove this.
//       
//       The old api is racy wrt. module removal, all users have been converted
//       to the netns-aware api.
//       
//       The old api pretended we still have global hooks but that has not been
//       true for a long time.
//
// So, we'll add our own nf_register_hook(), that pretends that global
// hooks exist.

#include <net/net_namespace.h>
#include <linux/netfilter.h>
#include <linux/errno.h>

static int nf_register_hook(struct nf_hook_ops *reg)
{
	struct net *net, *last;
	int ret = 0;

	for_each_net(net) {
		ret = nf_register_net_hook(net, reg);
		if (ret && ret != -ENOENT)
			goto rollback;
	}
	return 0;

rollback:
	last = net;
	for_each_net(net) {
		if (net == last)
			break;
		nf_unregister_net_hook(net, reg);
	}
	return ret;
}

static void nf_unregister_hook(struct nf_hook_ops *reg)
{
	struct net *net;

	for_each_net(net) {
		nf_unregister_net_hook(net, reg);
	}
}

#endif	// STAPCONF_NF_REGISTER_HOOK
#endif	// __NETFILTER_C__
