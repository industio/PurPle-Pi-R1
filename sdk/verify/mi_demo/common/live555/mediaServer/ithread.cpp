/******************************************************************************
*
* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : ithread.c                                            */
/*                                                                           */
/*  Description       : Contains abstraction for threads, mutex and semaphores*/
/*                                                                           */
/*  List of Functions :                                                      */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   Harish          Initial Version                      */
/*****************************************************************************/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/

#include <string.h>
#include "ithread.h"

#if defined(__MINGW32__)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if defined(WIN32) && !defined(__MINGW32__)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <Windows.h>
#include <WinBase.h>
#define SEM_MAX_COUNT       100
#define SEM_INCREMENT_COUNT 1

UWORD32 ithread_get_handle_size(void)
{
	return (sizeof(HANDLE));
}

UWORD32 ithread_get_mutex_lock_size(void)
{
	return (sizeof(HANDLE));
}

WORD32 ithread_create(void *thread_handle, void *attribute, void *strt, void *argument)
{
	HANDLE *ppv_thread_handle;
	HANDLE thread_handle_value;

	if (0 == thread_handle)
		return -1;

	ppv_thread_handle = (HANDLE *)thread_handle;
	thread_handle_value = (void *)CreateThread
	(NULL,                              /* Attributes      */
		1024 * 128,                        /* Stack size      */
		(LPTHREAD_START_ROUTINE)strt,      /* Thread function */
		argument,                          /* Parameters      */
		0,                                 /* Creation flags  */
		NULL);                             /* Thread ID       */
	*ppv_thread_handle = (HANDLE)thread_handle_value;

	return 0;
}

WORD32 ithread_join(void *thread_handle, void **val_ptr)
{
	HANDLE *ppv_thread_handle;
	HANDLE thread_handle_value;

	if (0 == thread_handle)
		return -1;

	ppv_thread_handle = (HANDLE *)thread_handle;
	thread_handle_value = *ppv_thread_handle;

	if (WAIT_OBJECT_0 == WaitForSingleObject(thread_handle_value, INFINITE))
	{
		CloseHandle(thread_handle_value);
	}

	return 0;
}

void ithread_exit(void *thread_handle)
{
	HANDLE *ppv_thread_handle;
	HANDLE thread_handle_value;
	DWORD thread_exit_code;

	if (0 == thread_handle)
		return;

	ppv_thread_handle = (HANDLE *)thread_handle;
	thread_handle_value = *ppv_thread_handle;
	/* Get exit code for thread. If the return value is 0, means thread is busy */
	if (0 != GetExitCodeThread(thread_handle_value, &thread_exit_code))
	{
		TerminateThread(thread_handle_value, thread_exit_code);
	}

	return;
}

WORD32 ithread_get_mutex_struct_size(void)
{
	return (sizeof(HANDLE));
}

WORD32 ithread_mutex_init(void *mutex)
{
	HANDLE *ppv_mutex_handle;
	HANDLE mutex_handle_value;

	if (0 == mutex)
		return -1;

	ppv_mutex_handle = (HANDLE *)mutex;
	mutex_handle_value = CreateSemaphore(NULL, 1, 1, NULL);
	*ppv_mutex_handle = mutex_handle_value;
	return 0;
}

WORD32 ithread_mutex_destroy(void *mutex)
{
	HANDLE *ppv_mutex_handle;
	HANDLE mutex_handle_value;

	if (0 == mutex)
		return -1;

	ppv_mutex_handle = (HANDLE *)mutex;
	mutex_handle_value = *ppv_mutex_handle;
	CloseHandle(mutex_handle_value);
	return 0;
}

WORD32 ithread_mutex_lock(void *mutex)
{
	HANDLE *ppv_mutex_handle;
	HANDLE mutex_handle_value;
	DWORD  result = 0;

	if (0 == mutex)
		return -1;

	ppv_mutex_handle = (HANDLE *)mutex;
	mutex_handle_value = *ppv_mutex_handle;
	result = WaitForSingleObject(mutex_handle_value, INFINITE);

	if (WAIT_OBJECT_0 == result)
		return 0;

	return 1;

}

WORD32 ithread_mutex_unlock(void *mutex)
{
	HANDLE *ppv_mutex_handle;
	HANDLE mutex_handle_value;
	DWORD  result = 0;

	if (0 == mutex)
		return -1;

	ppv_mutex_handle = (HANDLE *)mutex;
	mutex_handle_value = *ppv_mutex_handle;
	result = ReleaseSemaphore(mutex_handle_value, 1, NULL);

	if (0 == result)
		return -1;

	return 0;
}

void ithread_yield(void) 
{
	SwitchToThread();
	//Sleep(0);
}

void ithread_usleep(UWORD32 u4_time_us)
{
	UWORD32 u4_time_ms = u4_time_us / 1000;
	Sleep(u4_time_ms);
}

void ithread_msleep(UWORD32 u4_time_ms)
{
	Sleep(u4_time_ms);
}


void ithread_sleep(UWORD32 u4_time)
{
	UWORD32 u4_time_ms = u4_time * 1000;
	Sleep(u4_time_ms);
}

UWORD32 ithread_get_sem_struct_size(void)
{
	return (sizeof(HANDLE));
}

WORD32 ithread_sem_init(void *sem, WORD32 pshared, UWORD32 value)
{
	HANDLE *sem_handle = (HANDLE *)sem;
	HANDLE sem_handle_value;

	if (0 == sem)
		return -1;

	sem_handle_value = CreateSemaphore(NULL,  /* Security Attribute*/
		value,  /* Initial count     */
		SEM_MAX_COUNT, /* Max value         */
		NULL);        /* Name, not used    */
	*sem_handle = sem_handle_value;
	return 0;
}

WORD32 ithread_sem_post(void *sem)
{
	HANDLE *sem_handle = (HANDLE *)sem;
	HANDLE sem_handle_value;

	if (0 == sem)
		return -1;

	sem_handle_value = *sem_handle;

	/* Post on Semaphore by releasing the lock on mutex */
	if (ReleaseSemaphore(sem_handle_value, SEM_INCREMENT_COUNT, NULL))
		return 0;

	return -1;
}

WORD32 ithread_sem_wait(void *sem)
{
	DWORD          result = 0;
	HANDLE *sem_handle = (HANDLE *)sem;
	HANDLE sem_handle_value;

	if (0 == sem)
		return -1;

	sem_handle_value = *sem_handle;

	/* Wait on Semaphore object infinitly */
	result = WaitForSingleObject(sem_handle_value, INFINITE);

	/* If lock on semaphore is acquired, return SUCCESS */
	if (WAIT_OBJECT_0 == result)
		return 0;

	/* If call timeouts, return FAILURE */
	if (WAIT_TIMEOUT == result)
		return -1;

	return 0;
}

WORD32 ithread_sem_destroy(void *sem)
{
	HANDLE *sem_handle = (HANDLE *)sem;
	HANDLE sem_handle_value;

	if (0 == sem)
		return -1;

	sem_handle_value = *sem_handle;

	if (FALSE == CloseHandle(sem_handle_value))
	{
		return -1;
	}
	return 0;
}

WORD32 ithread_set_affinity(WORD32 core_id)
{
	int iRet=0;
	if ( core_id==0 ) {
		iRet = SetThreadAffinityMask(GetCurrentThread(),0x00000001);
	} else {
		iRet = SetThreadAffinityMask(GetCurrentThread(),0x0FFFFFFE);
	}
	return 0;
}

#else
#ifdef PTHREAD_AFFINITY
#define _GNU_SOURCE
#define __USE_GNU
#endif
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <unistd.h>	

UWORD32 ithread_get_handle_size(void)
{
    return sizeof(pthread_t);
}

UWORD32 ithread_get_mutex_lock_size(void)
{
    return sizeof(pthread_mutex_t);
}

WORD32 ithread_create(void *thread_handle, void *attribute, void *strt, void *argument)
{
    return pthread_create((pthread_t *)thread_handle, (const pthread_attr_t*)attribute, (void * (*)(void *))strt, argument);
}

WORD32 ithread_join(void *thread_handle, void **val_ptr)
{
    pthread_t *pthread_handle   = (pthread_t *)thread_handle;
    return pthread_join(*pthread_handle, val_ptr);
}

void ithread_exit(void *val_ptr)
{
    return pthread_exit(val_ptr);
}

WORD32 ithread_get_mutex_struct_size(void)
{
    return (sizeof(pthread_mutex_t));
}
WORD32 ithread_mutex_init(void *mutex)
{
    return pthread_mutex_init((pthread_mutex_t *)mutex, NULL);
}

WORD32 ithread_mutex_destroy(void *mutex)
{
    return pthread_mutex_destroy((pthread_mutex_t *)mutex);
}

WORD32 ithread_mutex_lock(void *mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *)mutex);
}

WORD32 ithread_mutex_unlock(void *mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

void ithread_yield(void)
{
    sched_yield();
}

void ithread_sleep(UWORD32 u4_time)
{
	struct timeval wait_timeval;
	wait_timeval.tv_sec = u4_time;
	wait_timeval.tv_usec = 0;
	select(0, NULL, NULL, NULL, &wait_timeval);
}

void ithread_msleep(UWORD32 u4_time_ms)
{
    struct timeval wait_timeval;
	wait_timeval.tv_sec = u4_time_ms/1000;
	wait_timeval.tv_usec = (u4_time_ms%1000)*1000;
	select(0, NULL, NULL, NULL, &wait_timeval);
}

void ithread_usleep(UWORD32 u4_time_us)
{
    struct timeval wait_timeval;
	wait_timeval.tv_sec = u4_time_us/1000000;
	wait_timeval.tv_usec = u4_time_us%1000000;
	select(0, NULL, NULL, NULL, &wait_timeval);
}

UWORD32 ithread_get_sem_struct_size(void)
{
    return (sizeof(sem_t));
}

WORD32 ithread_sem_init(void *sem, WORD32 pshared, UWORD32 value)
{
    return sem_init((sem_t *)sem, pshared, value);
}

WORD32 ithread_sem_post(void *sem)
{
    return sem_post((sem_t *)sem);
}

WORD32 ithread_sem_wait(void *sem)
{
    return sem_wait((sem_t *)sem);
}

WORD32 ithread_sem_destroy(void *sem)
{
    return sem_destroy((sem_t *)sem);
}

WORD32 ithread_set_affinity(WORD32 core_id)
{

#ifdef PTHREAD_AFFINITY
    cpu_set_t cpuset;
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t cur_thread = pthread_self();

    if(core_id >= num_cores)
        return -1;

    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    return pthread_setaffinity_np(cur_thread, sizeof(cpu_set_t), &cpuset);

#elif SYSCALL_AFFINITY
    WORD32 i4_sys_res;

    pid_t pid = gettid();


    i4_sys_res = syscall(__NR_sched_setaffinity, pid, sizeof(i4_mask), &i4_mask);
    if(i4_sys_res)
    {
        //WORD32 err;
        //err = errno;
        //perror("Error in setaffinity syscall PERROR : ");
        //LOG_ERROR("Error in the syscall setaffinity: mask=0x%x err=0x%x", i4_mask, i4_sys_res);
        return -1;
    }
#endif

    return core_id;
}

#endif
