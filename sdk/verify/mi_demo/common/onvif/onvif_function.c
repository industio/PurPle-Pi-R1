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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "onvif_function.h"

#define BUFSIZE 8192



int mst_netGetIp(char *pInterface, unsigned int *ip)
{
	int sock = 0;
	struct ifreq ifr;

	if((pInterface == NULL) || (*pInterface == '\0'))
	{
		printf("get ip: pInterface == NULL\r\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface,  strlen(pInterface)>IFNAMSIZ?IFNAMSIZ:strlen(pInterface));
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock <= 0)
	{
		printf("get ip: sock error, %s\r\n", strerror(errno));
		return -1;
	}

	((struct sockaddr_in*)&ifr.ifr_addr)->sin_family = PF_INET;
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		printf("1 get ip error: %s\r\n", strerror(errno));
		close(sock);
		return -1;
	}
	else
	{
		*ip = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
		printf("get ip(%d:%d:%d:%d) success!\r\n", (*ip)&0xff, (*ip>>8)&0xff, (*ip>>16)&0xff, (*ip>>24)&0xff);
	}
	close(sock);

	return 0;
}


int mst_GetMac(
    char *pInterface,
    unsigned char *pMac)
{
	struct ifreq ifreq;
	int sockfd = 0;
	unsigned char mac[6] = {0};

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("netGetMac socket");
		return -1;
	}

	strcpy(ifreq.ifr_name, pInterface);

	if(ioctl(sockfd, SIOCGIFHWADDR, &ifreq) < 0)
	{
		perror("netGetMac ioctl");
		close(sockfd);
		return -2;
	}

	memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);
	printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if(pMac != NULL)
	{
		memcpy(pMac, mac, 6);
	}

	close(sockfd);

	return 0;
}

int mst_GetV4Ip(
    char *pInterface,
    unsigned int *ip)
{
	int sock = 0;
	struct ifreq ifr;

	if((pInterface == NULL) || (*pInterface == '\0'))
	{
		printf("get ip: pInterface == NULL\r\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, IFNAMSIZ);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock <= 0)
	{
		printf("get ip: sock error, %s\r\n", strerror(errno));
		return -1;
	}

	((struct sockaddr_in*)&ifr.ifr_addr)->sin_family = PF_INET;
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		//printf("0 get ip error: %s\r\n", strerror(errno));
		close(sock);
		return -1;
	}
	else
	{
		*ip = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
		//printf("get ip(%d:%d:%d:%d) success!\r\n", (*ip)&0xff, (*ip>>8)&0xff, (*ip>>16)&0xff, (*ip>>24)&0xff);
	}
	close(sock);

	return 0;
}

int mst_GetV6Ip(
    char *pInterface,
    char *ip)
{
    char v6Ip[64];
    struct ifaddrs *ifap0, *ifap;
    struct sockaddr_in6 *addr6;

    if( pInterface == NULL || ip == NULL )
    {
        return -1;
    }

    if(getifaddrs(&ifap0))
    {
        return -1;
    }

    ifap = ifap0;

    for(;ifap != NULL; ifap = ifap->ifa_next)
    {
        addr6 = (struct sockaddr_in6 *)ifap->ifa_addr;

        if(ifap->ifa_addr == NULL)
            continue;

        if(ifap->ifa_addr->sa_family != AF_INET6 )
            continue;

        if(strcmp(pInterface, ifap->ifa_name) != 0)
            continue;

        if ((ifap->ifa_flags & IFF_UP) == 0)
            continue;

        if(IN6_IS_ADDR_MULTICAST(&addr6->sin6_addr))
            continue;

        if(!IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr)) //here now use the local address to test
            continue;

        if(IN6_IS_ADDR_LOOPBACK(&addr6->sin6_addr))
            continue;

        if(IN6_IS_ADDR_UNSPECIFIED(&addr6->sin6_addr))
            continue;

        if(IN6_IS_ADDR_SITELOCAL(&addr6->sin6_addr))
            continue;

        if(inet_ntop(ifap->ifa_addr->sa_family, (void *)&(addr6->sin6_addr), v6Ip, sizeof(v6Ip)))
        {
            strcpy(ip, v6Ip);
            freeifaddrs(ifap0);  // why here can not free?
            return 0;
        }
    }

    freeifaddrs(ifap0);   // why here can not free?

    return -1;
}

int mst_GetSysDataTime(
    onvif_system_date_time* tSysDataTime
)
{
    int  timeZone;
    time_t now;
    struct tm *tmutc, *tmlocal;

    time(&now);

    tmutc = gmtime(&now);

    tSysDataTime->tUtcTime.year   = tmutc->tm_year + 1900;
    tSysDataTime->tUtcTime.month  = tmutc->tm_mon + 1;
    tSysDataTime->tUtcTime.day    = tmutc->tm_mday;
    tSysDataTime->tUtcTime.hour   = tmutc->tm_hour;
    tSysDataTime->tUtcTime.minute = tmutc->tm_min;
    tSysDataTime->tUtcTime.second = tmutc->tm_sec;

    tmlocal = localtime(&now);

    tSysDataTime->tLocalTime.year   = tmlocal->tm_year + 1900;
    tSysDataTime->tLocalTime.month  = tmlocal->tm_mon + 1;
    tSysDataTime->tLocalTime.day    = tmlocal->tm_mday;
    tSysDataTime->tLocalTime.hour   = tmlocal->tm_hour;
    tSysDataTime->tLocalTime.minute = tmlocal->tm_min;
    tSysDataTime->tLocalTime.second = tmlocal->tm_sec;

    timeZone = (tSysDataTime->tLocalTime.hour + 24 - tSysDataTime->tUtcTime.hour)%24;
    sprintf(tSysDataTime->tTimeZone, "%s-%d:00:00",tmlocal->tm_zone, timeZone);

    return 0;
}

static int __ReadNlSock(
    int sockfd, char* pBuf,
    int seqNum, int pId)
{
    int readLen = 0, msgLen = 0;
    struct nlmsghdr* nlHdr = NULL;

    //printf("[%d] %s enter !\n", __LINE__,__func__);

    while (1)
    {
        if ( (readLen = recv(sockfd, pBuf, BUFSIZE - msgLen, 0)) < 0 )
        {
            return -1;
        }

        nlHdr = (struct nlmsghdr *)pBuf;

        if ( (NLMSG_OK(nlHdr, (unsigned int)readLen) == 0)
                || (nlHdr->nlmsg_type == NLMSG_ERROR) )
        {
            return -1;
        }

        if ( nlHdr->nlmsg_type == NLMSG_DONE )
        {
            break;
        }
        else
        {
            pBuf += readLen;
            msgLen += readLen;
        }

        if ( (nlHdr->nlmsg_flags & NLM_F_MULTI) == 0 )
        {
            break;
        }

        if ( (nlHdr->nlmsg_seq != (unsigned int)seqNum)
                || (nlHdr->nlmsg_pid != (unsigned int)pId) )
        {
            break;
        }
    }

    //printf("[%d] %s exit !\n", __LINE__,__func__);

    return msgLen;
}

static int __ParseRoutes(
    struct nlmsghdr* nlHdr,
    route_info* rtInfo,
    char* default_gateway )
{
    int rtLen = 0;
    struct in_addr dst;
    struct in_addr gate;
    struct rtmsg* rtMsg = NULL;
    struct rtattr* rtAttr = NULL;

    //printf("[%d] %s enter !\n", __LINE__,__func__);

    rtMsg = (struct rtmsg*)NLMSG_DATA(nlHdr);

    if ( (rtMsg->rtm_family != AF_INET)
            || (rtMsg->rtm_table != RT_TABLE_MAIN) )
    {
        return -1;
    }

    rtAttr = (struct rtattr*)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);
    for ( ; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen) )
    {
        switch (rtAttr->rta_type)
        {
        case RTA_OIF:
            if_indextoname(*(int*)RTA_DATA(rtAttr), rtInfo->ifName);
            break;
        case RTA_GATEWAY:
            rtInfo->gateWay = *(u_int*)RTA_DATA(rtAttr);
            break;
        case RTA_PREFSRC:
            rtInfo->srcAddr = *(u_int*)RTA_DATA(rtAttr);
            break;
        case RTA_DST:
            rtInfo->dstAddr = *(u_int*)RTA_DATA(rtAttr);
            break;
        }
    }

    dst.s_addr = rtInfo->dstAddr;
    if (strstr((char*)inet_ntoa(dst), "0.0.0.0"))
    {
        gate.s_addr = rtInfo->gateWay;
        strcpy(default_gateway, (char*)inet_ntoa(gate));

    }

    //printf("[%d] %s exit !\n", __LINE__,__func__);

    return 0;
}


int mst_GetDefaultGateway(
    char* pInterface,
    char* szDefaultGateway )
{
    int ret = -1;
    int len = 0, msgSeq = 0;

    static char szGatewayTemp[32] = {0};
    static char msgBuf[BUFSIZE] = {0};
    static route_info ri;
    route_info* rtInfo = &ri;

    struct nlmsghdr* nlMsg = NULL;
    //struct rtmsg* rtMsg = NULL;

    int sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if ( -1 == sock )
    {
        goto END;
    }

    nlMsg = (struct nlmsghdr*)msgBuf;
    //rtMsg = (struct rtmsg*)NLMSG_DATA(nlMsg);

    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlMsg->nlmsg_type = RTM_GETROUTE;
    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    nlMsg->nlmsg_seq = msgSeq++;
    nlMsg->nlmsg_pid = getpid();

    if ( send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0 )
    {
        goto END;
    }

    if ( (len = __ReadNlSock(sock, msgBuf, msgSeq, getpid())) < 0 )
    {
        goto END;
    }

    if ( rtInfo == NULL )
    {
        goto END;
    }

    for ( ; NLMSG_OK(nlMsg, (unsigned int)len); nlMsg = NLMSG_NEXT(nlMsg, len) )
    {
        memset(szGatewayTemp, 0, sizeof(szGatewayTemp));
        memset(rtInfo, 0, sizeof(route_info));
        if ( 0 == __ParseRoutes(nlMsg, rtInfo, szGatewayTemp) )
        {
            if ( strcmp(rtInfo->ifName, pInterface) == 0
                    && strcmp(szGatewayTemp, "0.0.0.0") != 0
                    && strlen(szGatewayTemp) > 0 )
            {
                strcpy(szDefaultGateway, szGatewayTemp);
                ret = 0;
            }
        }
    }

END:

    if ( sock != -1 )
    {
        close(sock);
        sock = -1;
    }

    return ret;
}

