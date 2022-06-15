#include <linux/mm.h>

//
// The following kernel commit changed the get_user_pages_remote()
// function signature:
//
// commit 5b56d49fc31dbb0487e14ead790fc81ca9fb2c99
// Author: Lorenzo Stoakes <lstoakes@gmail.com>
// Date:   Wed Dec 14 15:06:52 2016 -0800
// 
// mm: add locked parameter to get_user_pages_remote()
//        
// This changed the function signature from:
//
// long get_user_pages_remote(struct task_struct *tsk, struct mm_struct *mm,
//                            unsigned long start, unsigned long nr_pages,
//                            unsigned int gup_flags, struct page **pages,
//                            struct vm_area_struct **vmas);
//
// long get_user_pages_remote(struct task_struct *tsk, struct mm_struct *mm,
//                            unsigned long start, unsigned long nr_pages,
//                            unsigned int gup_flags, struct page **pages,
//                            struct vm_area_struct **vmas, int *locked);
//

long gupr_wrapper2(struct task_struct *tsk, struct mm_struct *mm,
                   unsigned long start, unsigned long nr_pages,
                   unsigned int gup_flags, struct page **pages,
                   struct vm_area_struct **vmas, int *locked)
{
    return get_user_pages_remote(tsk, mm, start, nr_pages, gup_flags,
				 pages, vmas, locked);
}
