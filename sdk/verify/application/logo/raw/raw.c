#include "raw.h"
#if defined(LOGO_WITH_RAW)
#include <stdlib.h>
#include <sys/stat.h>
#include <sched.h>
#include <pthread.h>
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

FILE * logofp =NULL;
char *frameBuffer  =NULL;
void *readTOPlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");

    fseek(logofp, 0, SEEK_SET);
    size_t n = fread(frameBuffer, (MI_U32)pos, 1, logofp);

}

void *readBTMlogo(void *pos)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(1, &set);

    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) != 0)
        errExit("sched_setaffinity");
    fseek(logofp, (MI_U32)pos, SEEK_SET);
    size_t n = fread(frameBuffer + (MI_U32)pos, (MI_U32)pos, 1, logofp);

}


int load_logo_raw(char* fbuffer,BITMAP* fb,FILE* fp)
{
    pthread_t toplogo, btmlogo;
    int ret;
    struct stat  fs;
    fstat(fileno(fp), &fs);
    logofp = fp;
    frameBuffer = fbuffer;
    ret = pthread_create(&toplogo, NULL, readTOPlogo, (void *)(fs.st_size / 2));
    ret = pthread_create(&btmlogo, NULL, readBTMlogo, (void *)(fs.st_size / 2));
    pthread_join(toplogo, NULL);
    pthread_join(btmlogo, NULL);

    return 0;
}
#endif
