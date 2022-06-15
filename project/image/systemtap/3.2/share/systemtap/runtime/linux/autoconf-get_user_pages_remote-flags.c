#include <linux/mm.h>

//
// The following kernel commit changed the get_user_pages_remote()
// function signature:
//
// commit 9beae1ea89305a9667ceaab6d0bf46a045ad71e7
// Author: Lorenzo Stoakes <lstoakes@gmail.com>
// Date:   Thu Oct 13 01:20:17 2016 +0100
//
//    mm: replace get_user_pages_remote() write/force parameters with gup_flags
//
//    This removes the 'write' and 'force' from get_user_pages_remote() and
//    replaces them with 'gup_flags' to make the use of FOLL_FORCE explicit in
//    callers as use of this flag can result in surprising behaviour (and
//    hence bugs) within the mm subsystem.
//
// This changed the function signature from:
//
// long get_user_pages_remote(struct task_struct *tsk, struct mm_struct *mm,
//                            unsigned long start, unsigned long nr_pages,
//                            int write, int force, struct page **pages,
//                            struct vm_area_struct **vmas);
//
// to:
//
// long get_user_pages_remote(struct task_struct *tsk, struct mm_struct *mm,
//                            unsigned long start, unsigned long nr_pages,
//                            unsigned int gup_flags, struct page **pages,
//                            struct vm_area_struct **vmas);
//

long gupr_wrapper(struct task_struct *tsk, struct mm_struct *mm,
		  unsigned long start, unsigned long nr_pages,
		  unsigned int gup_flags, struct page **pages,
		  struct vm_area_struct **vmas)
{
    return get_user_pages_remote(tsk, mm, start, nr_pages, gup_flags,
				 pages, vmas);
}
