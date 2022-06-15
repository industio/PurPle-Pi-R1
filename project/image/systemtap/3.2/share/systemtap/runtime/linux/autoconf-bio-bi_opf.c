#include <linux/bio.h>

/*
 * Handle the following kernel commit:
 *
 *   commit 1eff9d322a444245c67515edb52bc0eb68374aa8
 *   Author: Jens Axboe <axboe@fb.com>
 *   Date:   Fri Aug 5 15:35:16 2016 -0600
 *   
 *       block: rename bio bi_rw to bi_opf
 *       
 *       Since commit 63a4cc24867d, bio->bi_rw contains flags in the lower
 *       portion and the op code in the higher portions. This means that
 *       old code that relies on manually setting bi_rw is most likely
 *       going to be broken. Instead of letting that brokeness linger,
 *       rename the member, to force old and out-of-tree code to break
 *       at compile time instead of at runtime.
 *       
 *       No intended functional changes in this commit.
 */

void bar (void)
{
        struct bio foo;
	foo.bi_opf = 0;
        (void) foo;
}
