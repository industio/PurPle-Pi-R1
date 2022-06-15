#include <linux/fs.h>

// check for 4.6 inode patch which changed i_mutex to i_rwsem

struct inode i  __attribute__ ((unused)) = {.i_rwsem=__RWSEM_INITIALIZER(i.i_rwsem)};
