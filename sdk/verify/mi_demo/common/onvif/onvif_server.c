/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include "wsdd.h"
#include "soapH.h"
#include "onvif_function.h"
#include "onvif_server.h"
#include "stdsoap2.h"

static int     bExit = 0;
struct soap soapDiscovery;
struct soap soapHandle;

pthread_t threadDis;
pthread_t threadHdl;

int fETH_NAME = 0;
int fHasReboot = 0;


int file_exists(const char *file)
{
	if(!file)return 0;
	if( access(file,0) == 0 )
		return 1;
	return 0;
}

void* thread_discovery(void *arg)
{
	if(fHasReboot)sleep(2);
	bExit = 0;
	prctl(PR_SET_NAME, "ONVIF_Discovery");
    int count = 0;
    struct ip_mreq mcast;

    soap_init1(&soapDiscovery, SOAP_IO_UDP | SOAP_XML_IGNORENS);    // soap_init_LIBRARY_VERSION_REQUIRED_20821
    soap_set_namespaces(&soapDiscovery,  namespaces);

    printf("[%s][%d][%s][%s] start ServerSoap.version = %d \n", __FILE__, __LINE__, __TIME__, __func__, soapDiscovery.version);
	/*
    if(!soap_valid_socket(soap_bind(&soapDiscovery, NULL, ONVIF_LISTEN_PORT, 10)))
    {
        soap_print_fault(&soapDiscovery, stderr);
        bExit = 1;
		printf("jerry %s %d\n", __FUNCTION__, __LINE__);
//        exit(1);
    }
	*/
	while(!soap_valid_socket(soap_bind(&soapDiscovery, NULL, ONVIF_LISTEN_PORT, 10)))
    {
        sleep(1);
    }

	memset(&mcast, 0, sizeof(mcast));
    mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);

    printf("[%s][%d][%s][%s] start ServerSoap.version = %d \n", __FILE__, __LINE__, __TIME__, __func__, soapDiscovery.version);
    while(setsockopt(soapDiscovery.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
    {
        sleep(1);
        printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
//        exit(1);
    }

    while(!bExit)
    {
        printf("[%s][%d][%s][%s] start ServerSoap.version = %d \n", __FILE__, __LINE__, __TIME__, __func__, soapDiscovery.version);
        if(soap_serve(&soapDiscovery))
        {
            soap_print_fault(&soapDiscovery, stderr);
        }
        soap_destroy(&soapDiscovery);
        soap_end(&soapDiscovery);

        //customer address
        #if 1
        printf("RECEIVE count %d, connection from IP = %lu.%lu.%lu.%lu socket = %d \r\n",
        count,
        ((soapDiscovery.ip)>>24)&0xFF,
        ((soapDiscovery.ip)>>16)&0xFF,
        ((soapDiscovery.ip)>>8)&0xFF,
        (soapDiscovery.ip)&0xFF,
        (soapDiscovery.socket));

        count++;
	    #endif
    }

    soap_closesock(&soapDiscovery);
    soap_done(&soapDiscovery);

    //bExit = 1;
	return NULL;
    printf("[%s][%d][%s][%s] end \n", __FILE__, __LINE__, __TIME__, __func__);

}


void* thread_onvif_handle(void *arg)
{
    printf("[%s][%d][%s][%s] start \n", __FILE__, __LINE__, __TIME__, __func__);
    if(fHasReboot)sleep(2);
    bExit = 0;
	prctl(PR_SET_NAME, "ONVIF_Accept");
    soap_init(&soapHandle);
    soapHandle.send_timeout = 2;
	soapHandle.recv_timeout = 2;
	soapHandle.accept_timeout = 2;

    soap_set_namespaces(&soapHandle, namespaces);

    while(!soap_valid_socket(soap_bind(&soapHandle, NULL, ONVIF_SERVICE_PORT, 10)))
    {
    	sleep(1);
        //soap_print_fault(&soapHandle, stderr);
        //bExit = 1;
//        exit(1);
    }

    while(!bExit)
    {
    #if 1

		unsigned int localIp = 0;
    	mst_GetV4Ip(ETH_NAME, &localIp);
		if(0 != localIp)
		{
			fETH_NAME = 1;
		}
		else if(fETH_NAME && !fHasReboot)
		{
			fHasReboot = 1;
			fETH_NAME = 0;
			bExit = 1;
			pthread_kill(threadDis,0);
			soap_closesock(&soapDiscovery);
    		soap_done(&soapDiscovery);
			pthread_create(&threadDis,NULL,&thread_discovery,NULL);
			pthread_create(&threadHdl,NULL,&thread_onvif_handle,NULL);
			break;
		}

    	if ( soap_accept(&soapHandle) == SOAP_INVALID_SOCKET ) {
			continue;
		}
    #else
        if(soap_accept(&soapHandle) < 0)
        {
        	printf("1111%s LINE=%d\n", __FUNCTION__, __LINE__);
            soap_print_fault(&soapHandle, stderr);
            bExit = 1;
//            exit(1);
        }
	#endif
        soap_serve(&soapHandle);
        soap_end(&soapHandle);
    }

    soap_closesock(&soapHandle);
    soap_done(&soapHandle);
    //bExit = 1;
	return NULL;
    printf("[%s][%d][%s][%s] end \n", __FILE__, __LINE__, __TIME__, __func__);
}

void MST_ONVIF_Init()
{
    char cmd[256];
    //unsigned int localIp = 0;

    //mst_netGetIp(ETH_NAME, &localIp); //eth2 deside to the real net

    sprintf(cmd, "route add -net 239.255.255.250 netmask 255.255.255.255 %s", ETH_NAME);
    system(cmd);

    // memset(cmd, 0, 256);

    //sprintf(cmd, "route add default gw %s dev %s", inet_ntoa(*((struct in_addr *)&localIp)),ETH_NAME);
    //system(cmd);
}



void MST_ONVIF_StartTask()
{
    // printf("[%s][%d][%s][%s] start \n", __FILE__, __LINE__, __TIME__, __func__);
      bExit = 0;
      pthread_create(&threadDis,NULL,&thread_discovery,NULL);
      pthread_create(&threadHdl,NULL,&thread_onvif_handle,NULL);


   // printf("[%s][%d][%s][%s] end \n", __FILE__, __LINE__, __TIME__, __func__);
}

void MST_ONVIF_StopTask()
{
    bExit = 1;
	#if 1
	soap_closesock(&soapDiscovery);
    soap_done(&soapDiscovery);
	printf("onvif stop %s LINE=%d\n", __FUNCTION__, __LINE__);

	soap_closesock(&soapHandle);
    soap_done(&soapHandle);
	printf("onvif stop %s LINE=%d\n", __FUNCTION__, __LINE__);

	pthread_join(threadDis, NULL);
	printf("onvif stop %s LINE=%d\n", __FUNCTION__, __LINE__);

	pthread_join(threadHdl, NULL);
	printf("onvif stop %s LINE=%d\n", __FUNCTION__, __LINE__);
	#else
    struct timeval wait_timeval;

    wait_timeval.tv_sec = 0;
    wait_timeval.tv_usec = 500 *1000;
    select(0, NULL, NULL, NULL, &wait_timeval);
    pthread_cancel(threadDis);
    pthread_cancel(threadHdl);
	#endif
}

