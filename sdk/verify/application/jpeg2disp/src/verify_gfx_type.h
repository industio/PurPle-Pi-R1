#include <time.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

struct timespec ts1, ts2;
double posix_dur;
#define JDEC_PERF(ts1, ts2,cnt)                                                                                \
    {                                                                                                      \
        static int count = cnt;                                                                            \
        static double dur = 0.0;                                                                           \
        clock_gettime(CLOCK_MONOTONIC, &ts2);                                                     \
        posix_dur = 1000.0 * ts2.tv_sec + 1e-6 * ts2.tv_nsec - (1000.0 * ts1.tv_sec + 1e-6 * ts1.tv_nsec); \
        if (unlikely(dur - 0.0 < 0.001))                                                                   \
        {                                                                                                  \
            dur = posix_dur;                                                                               \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            dur = (dur + posix_dur) / 2;                                                                   \
        }                                                                                                  \
        count--;                                                                                           \
        if (unlikely(count == 0))                                                                          \
        {                                                                                                  \
            printf("CPU time used (%s->%d): %.2f ms\n", __FUNCTION__,__LINE__, dur);                       \
            count = cnt;                                                                                   \
            dur = 0.0;                                                                                     \
        }                                                                                                  \
        clock_gettime(CLOCK_MONOTONIC, &ts1);                                                     \
    }

