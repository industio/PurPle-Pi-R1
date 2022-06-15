/***********************************************************************
** Copyright (C) GUOG. All rights reserved.
** Author			: guog
** Date				: 2013.09.17
** Name				: onvif_server_interface.c
** Version			: 1.0
** Description			: 定义onvif服务端需要调用的接口
** Modify Record		:
1:Creat on 2013.09.17

***********************************************************************/

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

#include "soapH.h"
#include "soapStub.h"
#include "onvif_function.h"


#define LARGE_INFO_LEN   1024
#define COMMN_INFO_LEN   512
#define SMALL_INFO_LEN   128

#define ONVIF_SERVER_CALL()    printf("onvifs: call %s, path=%s\r\n", __FUNCTION__, soap->path)

#define ONVIF_RETURN_OK(soap, namespaces)   \
	ONVIF_SERVER_CALL();    \
	if(namespaces!=NULL) soap_set_namespaces(soap, namespaces);  \
	return SOAP_OK;

#define ONVIF_NOTSUPPORTED_FUNC(soap, namespaces)   \
	ONVIF_SERVER_CALL();    \
	if(namespaces!=NULL) soap_set_namespaces(soap, namespaces);  \
	return soap_receiver_fault_subcode(soap, "ter:ActionNotSupported", "Optional Action Not Implemented ", "The requested action is optional and is not implemented by the device.")

//onvif interface


static inline int onvif_receiver_fault_subcode_oom(struct soap *soap)
{
	return soap_receiver_fault_subcode(soap, "ter:OutofMemory", "Out of Memory", "The device does not have sufficient memory to complete the action.");
}


/******************************函数定义******************************/

//填充onvif服务端接口

/******************************************************************************\
 *                                                                            *
 * Server-Side Operations                                                     *
 *                                                                            *
\******************************************************************************/

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap* soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap* soap, struct wsdd__HelloType *wsdd__Hello){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap* soap, struct wsdd__ByeType *wsdd__Bye){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

/*
SOAP_FMAC5 int SOAP_FMAC6  __wsdd__Probe(struct soap* soap, struct wsdd__ProbeType *wsdd__Probe)
{

    #define MACH_ADDR_LENGTH 6
    #define INFO_LENGTH 512
    #define LARGE_INFO_LENGTH 1024
    #define SMALL_INFO_LENGTH 512

    printf("[%d] __wsdd__Probe start !\n", __LINE__);

    unsigned char macaddr[6] = {0};
    char _IPAddr[INFO_LENGTH] = {0};
    char _HwId[1024] = {0};

    wsdd__ProbeMatchesType ProbeMatches;
    ProbeMatches.ProbeMatch = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
    ProbeMatches.ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
    ProbeMatches.ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
    ProbeMatches.ProbeMatch->Scopes = (struct wsdd__ScopesType*)soap_malloc(soap,sizeof(struct wsdd__ScopesType));
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties = (struct wsa__ReferencePropertiesType*)soap_malloc(soap,sizeof(struct wsa__ReferencePropertiesType));
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters = (struct wsa__ReferenceParametersType*)soap_malloc(soap,sizeof(struct wsa__ReferenceParametersType));
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName = (struct wsa__ServiceNameType*)soap_malloc(soap,sizeof(struct wsa__ServiceNameType));
    ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType = (char **)soap_malloc(soap, sizeof(char *) * SMALL_INFO_LENGTH);
    ProbeMatches.ProbeMatch->wsa__EndpointReference.__any = (char **)soap_malloc(soap, sizeof(char*) * SMALL_INFO_LENGTH);
    ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
    ProbeMatches.ProbeMatch->wsa__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);

    netGetMac("eth0", macaddr); //eth0  根据实际情况填充
    macaddr[0]=0x01;macaddr[1]=0x01;macaddr[2]=0x01;macaddr[3]=0x01;macaddr[4]=0x01;macaddr[5]=0x01;
    sprintf(_HwId,"urn:uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X",macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);


    unsigned int localIp = 0;
    netGetIp("eth0", &localIp); //eth0 根据实际情况填充
    sprintf(_IPAddr, "http://%s/onvif/device_service", inet_ntoa(*((struct in_addr *)&localIp)));
    printf("[%d] _IPAddr ==== %s\n", __LINE__, _IPAddr);

    ProbeMatches.__sizeProbeMatch = 1;
    ProbeMatches.ProbeMatch->Scopes->__item =(char *)soap_malloc(soap, 1024);
    memset(ProbeMatches.ProbeMatch->Scopes->__item,0,sizeof(ProbeMatches.ProbeMatch->Scopes->__item));

    //Scopes MUST BE
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter");

    ProbeMatches.ProbeMatch->Scopes->MatchBy = NULL;
    strcpy(ProbeMatches.ProbeMatch->XAddrs, _IPAddr);
    strcpy(ProbeMatches.ProbeMatch->Types, wsdd__Probe->Types);
    printf("wsdd__Probe->Types=%s\n",wsdd__Probe->Types);
    ProbeMatches.ProbeMatch->MetadataVersion = 1;

    //ws-discovery规定 为可选项
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__size = 0;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__any = NULL;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__size = 0;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__any = NULL;

    ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
    //ws-discovery规定 为可选项
    strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0], "ttl");
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__item = NULL;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->PortName = NULL;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__anyAttribute = NULL;
    ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
    strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0], "Any");
    strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute, "Attribute");
    ProbeMatches.ProbeMatch->wsa__EndpointReference.__size = 0;
    strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.Address, _HwId);

    soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
    soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
    soap->header->wsa__RelatesTo = (struct wsa__Relationship*)soap_malloc(soap, sizeof(struct wsa__Relationship));
    soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
    soap->header->wsa__RelatesTo->RelationshipType = NULL;
    soap->header->wsa__RelatesTo->__anyAttribute = NULL;

    soap->header->wsa__MessageID =(char *)soap_malloc(soap, sizeof(char) * INFO_LENGTH);
    strcpy(soap->header->wsa__MessageID,_HwId+4);

        if (SOAP_OK == soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &ProbeMatches))
        {
        printf("send ProbeMatches success !\n");
        return SOAP_OK;
    }

    printf("[%d] soap error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));

    return soap->error;;

}
*/
#define MACH_ADDR_LENGTH 6
#define INFO_LENGTH 512
#define LARGE_INFO_LENGTH 1024
#define SMALL_INFO_LENGTH 512


SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ProbeMatches2(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{	struct __wsdd__ProbeMatches soap_tmp___wsdd__ProbeMatches;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches";
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__ProbeMatches.wsdd__ProbeMatches = wsdd__ProbeMatches;
	soap_serializeheader(soap);
	soap_serialize___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_url(soap, soap_endpoint, NULL), soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

int  __wsdd__Probe(struct soap* soap, struct wsdd__ProbeType *wsdd__Probe)
{
	char *ethName = ETH_NAME;
    unsigned int  V4Addr = 0;
    char V6Addr[80];
    unsigned char MacAddr[6];
    char HwId[SMALL_INFO_LEN];
    char IPAddr[SMALL_INFO_LEN];
	//char p2pKey[32];
	//char p2pItem[SMALL_INFO_LEN];
    printf("[%d] %s enter !\n", __LINE__,__func__);

    wsdd__ProbeMatchesType ProbeMatches;

    ProbeMatches.__sizeProbeMatch = 1;
    ProbeMatches.ProbeMatch = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));

    ProbeMatches.ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    ProbeMatches.ProbeMatch->Scopes = (struct wsdd__ScopesType*)soap_malloc(soap,sizeof(struct wsdd__ScopesType));
    ProbeMatches.ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    ProbeMatches.ProbeMatch->MetadataVersion = 1;
    strcpy(ProbeMatches.ProbeMatch->Types, wsdd__Probe->Types);
    //printf("wsdd__Probe->Types=%s\n",wsdd__Probe->Types);
//    FILE *fp;
//	fp = fopen("/ext/demo/p2p.key","r");
//	if(fp == NULL)
//	{
//		printf("open p2p.key fail\n");
		//return 0;
//	}

//	fscanf(fp,"%s",p2pKey);
//	sprintf(p2pItem,"onvif://www.onvif.org/p2pkey/%s ",p2pKey);
	//printf("p2pItem[%s]\n",p2pItem);

    ProbeMatches.ProbeMatch->Scopes->__item =(char *)soap_malloc(soap, LARGE_INFO_LEN);
    memset(ProbeMatches.ProbeMatch->Scopes->__item,0,LARGE_INFO_LEN);
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter ");
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/video_encoder ");
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/audio_encoder ");
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/ptz ");
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/hardware/10001 ");
	strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/name/Myipc ");
  //  strcat(ProbeMatches.ProbeMatch->Scopes->__item, p2pItem);
    strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/location/city/shenzhen ");
    ProbeMatches.ProbeMatch->Scopes->MatchBy = NULL;

    if(mst_GetV4Ip(ethName, &V4Addr)== 0)
    {
        sprintf(IPAddr, "http://%s:%d/onvif/device_service", inet_ntoa(*((struct in_addr *)&V4Addr)), ONVIF_SERVICE_PORT);
        //sprintf(_IPAddr, "http://%03d.%03d.%1d.%03d/onvif/device_service", 172, 21, 16, 162);
        strcpy(ProbeMatches.ProbeMatch->XAddrs, IPAddr);
        printf("[%d] _IPAddr ==== %s  [%s]\n", __LINE__, IPAddr,ETH_NAME);
    }
	else
	{
		//ethName = ETH_NAME == ethName ? ETH_NAME2 : ETH_NAME;
		ethName = (strcmp(ETH_NAME, "ethName") ? ETH_NAME2 : ETH_NAME);
		if(mst_GetV4Ip(ethName, &V4Addr)== 0)
		{
			sprintf(IPAddr, "http://%s:%d/onvif/device_service", inet_ntoa(*((struct in_addr *)&V4Addr)), ONVIF_SERVICE_PORT);
			//sprintf(_IPAddr, "http://%03d.%03d.%1d.%03d/onvif/device_service", 172, 21, 16, 162);
			strcpy(ProbeMatches.ProbeMatch->XAddrs, IPAddr);
			printf("[%d] _IPAddr ==== %s,[%s]\n", __LINE__, IPAddr,ETH_NAME2);
		}
	}
    if(mst_GetV6Ip(ETH_NAME, V6Addr) == 0)
    {
        sprintf(IPAddr, " http://[%s:%d]/onvif/device_service", V6Addr, ONVIF_SERVICE_PORT);
        strcat(ProbeMatches.ProbeMatch->XAddrs, IPAddr);
        printf("[%d] _IPAddr ==== %s\n", __LINE__, IPAddr);
    }

    soap_default_wsa__EndpointReferenceType(soap,&ProbeMatches.ProbeMatch->wsa__EndpointReference);
    mst_GetMac(ETH_NAME, MacAddr); //eth2  deside to the real net
    ProbeMatches.ProbeMatch->wsa__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    //macaddr[0]=0x01;macaddr[1]=0x01;macaddr[2]=0x01;macaddr[3]=0x01;macaddr[4]=0x01;macaddr[5]=0x01;
    sprintf(HwId,"urn:uuid:2419d68a-2dd2-21b2-a205-%02x%02x%02x%02x%02x%02x",MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.Address, HwId);
    printf("[%d] HwId ==== %s\n", __LINE__, HwId);

    //ws-discovery option item
    //ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType = (char **)soap_malloc(soap, sizeof(char *) * SMALL_INFO_LEN);
    //ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    //strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0], "ttl");

    //this two line are option
    if(NULL == soap->header)
		return SOAP_ERR;
    soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
    soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
    soap->header->wsa__RelatesTo = (struct wsa__Relationship*)soap_malloc(soap, sizeof(struct wsa__Relationship));
    soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
    soap->header->wsa__RelatesTo->RelationshipType = NULL;
    soap->header->wsa__RelatesTo->__anyAttribute = NULL;

    soap->header->wsa__MessageID =(char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(soap->header->wsa__MessageID,HwId+4);

    // send over current socket as HTTP OK response:
    //the second parameter must http

    soap_send___wsdd__ProbeMatches2(soap, "http://", NULL, &ProbeMatches);

    printf("[%d] %s exit !\n", __LINE__,__func__);
	//fclose(fp);
	//fp = NULL;
    return SOAP_OK;
}



SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap* soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
    //{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap* soap, struct wsdd__ResolveType *wsdd__Resolve){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap* soap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault_bak(struct soap* soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __ns1__GetSupportedActions(struct soap* soap, struct _ns1__GetSupportedActions *ns1__GetSupportedActions, struct _ns1__GetSupportedActionsResponse *ns1__GetSupportedActionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__GetActions(struct soap* soap, struct _ns1__GetActions *ns1__GetActions, struct _ns1__GetActionsResponse *ns1__GetActionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__CreateActions(struct soap* soap, struct _ns1__CreateActions *ns1__CreateActions, struct _ns1__CreateActionsResponse *ns1__CreateActionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__DeleteActions(struct soap* soap, struct _ns1__DeleteActions *ns1__DeleteActions, struct _ns1__DeleteActionsResponse *ns1__DeleteActionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__ModifyActions(struct soap* soap, struct _ns1__ModifyActions *ns1__ModifyActions, struct _ns1__ModifyActionsResponse *ns1__ModifyActionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__GetServiceCapabilities(struct soap* soap, struct _ns1__GetServiceCapabilities *ns1__GetServiceCapabilities, struct _ns1__GetServiceCapabilitiesResponse *ns1__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__GetActionTriggers(struct soap* soap, struct _ns1__GetActionTriggers *ns1__GetActionTriggers, struct _ns1__GetActionTriggersResponse *ns1__GetActionTriggersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__CreateActionTriggers(struct soap* soap, struct _ns1__CreateActionTriggers *ns1__CreateActionTriggers, struct _ns1__CreateActionTriggersResponse *ns1__CreateActionTriggersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__DeleteActionTriggers(struct soap* soap, struct _ns1__DeleteActionTriggers *ns1__DeleteActionTriggers, struct _ns1__DeleteActionTriggersResponse *ns1__DeleteActionTriggersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__ModifyActionTriggers(struct soap* soap, struct _ns1__ModifyActionTriggers *ns1__ModifyActionTriggers, struct _ns1__ModifyActionTriggersResponse *ns1__ModifyActionTriggersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetServiceCapabilities(struct soap* soap, struct _ns2__GetServiceCapabilities *ns2__GetServiceCapabilities, struct _ns2__GetServiceCapabilitiesResponse *ns2__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetAccessPointInfoList(struct soap* soap, struct _ns2__GetAccessPointInfoList *ns2__GetAccessPointInfoList, struct _ns2__GetAccessPointInfoListResponse *ns2__GetAccessPointInfoListResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetAccessPointInfo(struct soap* soap, struct _ns2__GetAccessPointInfo *ns2__GetAccessPointInfo, struct _ns2__GetAccessPointInfoResponse *ns2__GetAccessPointInfoResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetAreaInfoList(struct soap* soap, struct _ns2__GetAreaInfoList *ns2__GetAreaInfoList, struct _ns2__GetAreaInfoListResponse *ns2__GetAreaInfoListResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetAreaInfo(struct soap* soap, struct _ns2__GetAreaInfo *ns2__GetAreaInfo, struct _ns2__GetAreaInfoResponse *ns2__GetAreaInfoResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__GetAccessPointState(struct soap* soap, struct _ns2__GetAccessPointState *ns2__GetAccessPointState, struct _ns2__GetAccessPointStateResponse *ns2__GetAccessPointStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__EnableAccessPoint(struct soap* soap, struct _ns2__EnableAccessPoint *ns2__EnableAccessPoint, struct _ns2__EnableAccessPointResponse *ns2__EnableAccessPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__DisableAccessPoint(struct soap* soap, struct _ns2__DisableAccessPoint *ns2__DisableAccessPoint, struct _ns2__DisableAccessPointResponse *ns2__DisableAccessPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns2__ExternalAuthorization(struct soap* soap, struct _ns2__ExternalAuthorization *ns2__ExternalAuthorization, struct _ns2__ExternalAuthorizationResponse *ns2__ExternalAuthorizationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetServiceCapabilities(struct soap* soap, struct _ns4__GetServiceCapabilities *ns4__GetServiceCapabilities, struct _ns4__GetServiceCapabilitiesResponse *ns4__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetDoorInfoList(struct soap* soap, struct _ns4__GetDoorInfoList *ns4__GetDoorInfoList, struct _ns4__GetDoorInfoListResponse *ns4__GetDoorInfoListResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetDoorInfo(struct soap* soap, struct _ns4__GetDoorInfo *ns4__GetDoorInfo, struct _ns4__GetDoorInfoResponse *ns4__GetDoorInfoResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__GetDoorState(struct soap* soap, struct _ns4__GetDoorState *ns4__GetDoorState, struct _ns4__GetDoorStateResponse *ns4__GetDoorStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__AccessDoor(struct soap* soap, struct _ns4__AccessDoor *ns4__AccessDoor, struct _ns4__AccessDoorResponse *ns4__AccessDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__LockDoor(struct soap* soap, struct _ns4__LockDoor *ns4__LockDoor, struct _ns4__LockDoorResponse *ns4__LockDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__UnlockDoor(struct soap* soap, struct _ns4__UnlockDoor *ns4__UnlockDoor, struct _ns4__UnlockDoorResponse *ns4__UnlockDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__BlockDoor(struct soap* soap, struct _ns4__BlockDoor *ns4__BlockDoor, struct _ns4__BlockDoorResponse *ns4__BlockDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__LockDownDoor(struct soap* soap, struct _ns4__LockDownDoor *ns4__LockDownDoor, struct _ns4__LockDownDoorResponse *ns4__LockDownDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__LockDownReleaseDoor(struct soap* soap, struct _ns4__LockDownReleaseDoor *ns4__LockDownReleaseDoor, struct _ns4__LockDownReleaseDoorResponse *ns4__LockDownReleaseDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__LockOpenDoor(struct soap* soap, struct _ns4__LockOpenDoor *ns4__LockOpenDoor, struct _ns4__LockOpenDoorResponse *ns4__LockOpenDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__LockOpenReleaseDoor(struct soap* soap, struct _ns4__LockOpenReleaseDoor *ns4__LockOpenReleaseDoor, struct _ns4__LockOpenReleaseDoorResponse *ns4__LockOpenReleaseDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns4__DoubleLockDoor(struct soap* soap, struct _ns4__DoubleLockDoor *ns4__DoubleLockDoor, struct _ns4__DoubleLockDoorResponse *ns4__DoubleLockDoorResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetServiceCapabilities(struct soap* soap, struct _ns5__GetServiceCapabilities *ns5__GetServiceCapabilities, struct _ns5__GetServiceCapabilitiesResponse *ns5__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__CreateRSAKeyPair(struct soap* soap, struct _ns5__CreateRSAKeyPair *ns5__CreateRSAKeyPair, struct _ns5__CreateRSAKeyPairResponse *ns5__CreateRSAKeyPairResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetKeyStatus(struct soap* soap, struct _ns5__GetKeyStatus *ns5__GetKeyStatus, struct _ns5__GetKeyStatusResponse *ns5__GetKeyStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetPrivateKeyStatus(struct soap* soap, struct _ns5__GetPrivateKeyStatus *ns5__GetPrivateKeyStatus, struct _ns5__GetPrivateKeyStatusResponse *ns5__GetPrivateKeyStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetAllKeys(struct soap* soap, struct _ns5__GetAllKeys *ns5__GetAllKeys, struct _ns5__GetAllKeysResponse *ns5__GetAllKeysResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__DeleteKey(struct soap* soap, struct _ns5__DeleteKey *ns5__DeleteKey, struct _ns5__DeleteKeyResponse *ns5__DeleteKeyResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__CreatePKCS10CSR(struct soap* soap, struct _ns5__CreatePKCS10CSR *ns5__CreatePKCS10CSR, struct _ns5__CreatePKCS10CSRResponse *ns5__CreatePKCS10CSRResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__CreateSelfSignedCertificate(struct soap* soap, struct _ns5__CreateSelfSignedCertificate *ns5__CreateSelfSignedCertificate, struct _ns5__CreateSelfSignedCertificateResponse *ns5__CreateSelfSignedCertificateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__UploadCertificate(struct soap* soap, struct _ns5__UploadCertificate *ns5__UploadCertificate, struct _ns5__UploadCertificateResponse *ns5__UploadCertificateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetCertificate(struct soap* soap, struct _ns5__GetCertificate *ns5__GetCertificate, struct _ns5__GetCertificateResponse *ns5__GetCertificateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetAllCertificates(struct soap* soap, struct _ns5__GetAllCertificates *ns5__GetAllCertificates, struct _ns5__GetAllCertificatesResponse *ns5__GetAllCertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__DeleteCertificate(struct soap* soap, struct _ns5__DeleteCertificate *ns5__DeleteCertificate, struct _ns5__DeleteCertificateResponse *ns5__DeleteCertificateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__CreateCertificationPath(struct soap* soap, struct _ns5__CreateCertificationPath *ns5__CreateCertificationPath, struct _ns5__CreateCertificationPathResponse *ns5__CreateCertificationPathResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetCertificationPath(struct soap* soap, struct _ns5__GetCertificationPath *ns5__GetCertificationPath, struct _ns5__GetCertificationPathResponse *ns5__GetCertificationPathResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetAllCertificationPaths(struct soap* soap, struct _ns5__GetAllCertificationPaths *ns5__GetAllCertificationPaths, struct _ns5__GetAllCertificationPathsResponse *ns5__GetAllCertificationPathsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__DeleteCertificationPath(struct soap* soap, struct _ns5__DeleteCertificationPath *ns5__DeleteCertificationPath, struct _ns5__DeleteCertificationPathResponse *ns5__DeleteCertificationPathResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__AddServerCertificateAssignment(struct soap* soap, struct _ns5__AddServerCertificateAssignment *ns5__AddServerCertificateAssignment, struct _ns5__AddServerCertificateAssignmentResponse *ns5__AddServerCertificateAssignmentResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__RemoveServerCertificateAssignment(struct soap* soap, struct _ns5__RemoveServerCertificateAssignment *ns5__RemoveServerCertificateAssignment, struct _ns5__RemoveServerCertificateAssignmentResponse *ns5__RemoveServerCertificateAssignmentResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__ReplaceServerCertificateAssignment(struct soap* soap, struct _ns5__ReplaceServerCertificateAssignment *ns5__ReplaceServerCertificateAssignment, struct _ns5__ReplaceServerCertificateAssignmentResponse *ns5__ReplaceServerCertificateAssignmentResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __ns5__GetAssignedServerCertificates(struct soap* soap, struct _ns5__GetAssignedServerCertificates *ns5__GetAssignedServerCertificates, struct _ns5__GetAssignedServerCertificatesResponse *ns5__GetAssignedServerCertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetServiceCapabilities(struct soap* soap, struct _tad__GetServiceCapabilities *tad__GetServiceCapabilities, struct _tad__GetServiceCapabilitiesResponse *tad__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineControl(struct soap* soap, struct _tad__DeleteAnalyticsEngineControl *tad__DeleteAnalyticsEngineControl, struct _tad__DeleteAnalyticsEngineControlResponse *tad__DeleteAnalyticsEngineControlResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineControl(struct soap* soap, struct _tad__CreateAnalyticsEngineControl *tad__CreateAnalyticsEngineControl, struct _tad__CreateAnalyticsEngineControlResponse *tad__CreateAnalyticsEngineControlResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineControl(struct soap* soap, struct _tad__SetAnalyticsEngineControl *tad__SetAnalyticsEngineControl, struct _tad__SetAnalyticsEngineControlResponse *tad__SetAnalyticsEngineControlResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControl(struct soap* soap, struct _tad__GetAnalyticsEngineControl *tad__GetAnalyticsEngineControl, struct _tad__GetAnalyticsEngineControlResponse *tad__GetAnalyticsEngineControlResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineControls(struct soap* soap, struct _tad__GetAnalyticsEngineControls *tad__GetAnalyticsEngineControls, struct _tad__GetAnalyticsEngineControlsResponse *tad__GetAnalyticsEngineControlsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngine(struct soap* soap, struct _tad__GetAnalyticsEngine *tad__GetAnalyticsEngine, struct _tad__GetAnalyticsEngineResponse *tad__GetAnalyticsEngineResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngines(struct soap* soap, struct _tad__GetAnalyticsEngines *tad__GetAnalyticsEngines, struct _tad__GetAnalyticsEnginesResponse *tad__GetAnalyticsEnginesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetVideoAnalyticsConfiguration(struct soap* soap, struct _tad__SetVideoAnalyticsConfiguration *tad__SetVideoAnalyticsConfiguration, struct _tad__SetVideoAnalyticsConfigurationResponse *tad__SetVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__SetAnalyticsEngineInput(struct soap* soap, struct _tad__SetAnalyticsEngineInput *tad__SetAnalyticsEngineInput, struct _tad__SetAnalyticsEngineInputResponse *tad__SetAnalyticsEngineInputResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInput(struct soap* soap, struct _tad__GetAnalyticsEngineInput *tad__GetAnalyticsEngineInput, struct _tad__GetAnalyticsEngineInputResponse *tad__GetAnalyticsEngineInputResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsEngineInputs(struct soap* soap, struct _tad__GetAnalyticsEngineInputs *tad__GetAnalyticsEngineInputs, struct _tad__GetAnalyticsEngineInputsResponse *tad__GetAnalyticsEngineInputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsDeviceStreamUri(struct soap* soap, struct _tad__GetAnalyticsDeviceStreamUri *tad__GetAnalyticsDeviceStreamUri, struct _tad__GetAnalyticsDeviceStreamUriResponse *tad__GetAnalyticsDeviceStreamUriResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetVideoAnalyticsConfiguration(struct soap* soap, struct _tad__GetVideoAnalyticsConfiguration *tad__GetVideoAnalyticsConfiguration, struct _tad__GetVideoAnalyticsConfigurationResponse *tad__GetVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__CreateAnalyticsEngineInputs(struct soap* soap, struct _tad__CreateAnalyticsEngineInputs *tad__CreateAnalyticsEngineInputs, struct _tad__CreateAnalyticsEngineInputsResponse *tad__CreateAnalyticsEngineInputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__DeleteAnalyticsEngineInputs(struct soap* soap, struct _tad__DeleteAnalyticsEngineInputs *tad__DeleteAnalyticsEngineInputs, struct _tad__DeleteAnalyticsEngineInputsResponse *tad__DeleteAnalyticsEngineInputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tad__GetAnalyticsState(struct soap* soap, struct _tad__GetAnalyticsState *tad__GetAnalyticsState, struct _tad__GetAnalyticsStateResponse *tad__GetAnalyticsStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap* soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap* soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap* soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap* soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap* soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities(struct soap* soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap* soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules(struct soap* soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules(struct soap* soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap* soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules(struct soap* soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tdn__Hello(struct soap* soap, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType *tdn__HelloResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Bye(struct soap* soap, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType *tdn__ByeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tdn__Probe(struct soap* soap, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType *tdn__ProbeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap* soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
	int i = 0;
    int ServiceSize = 2;
    struct tds__Service *tService;

    unsigned int localIp = 0;
    char IPAddr[COMMN_INFO_LEN];

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetV4Ip(ETH_NAME, &localIp); //eth2 deside to the real net

    tService = (struct tds__Service *)soap_malloc(soap, sizeof(struct tds__Service)*ServiceSize);

    for(i = 0; i < ServiceSize; i++)
    {
        soap_default_tds__Service(soap,&tService[i]);

        tService[i].Namespace = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
        tService[i].XAddr = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
        tService[i].Version = (struct tt__OnvifVersion *)soap_malloc(soap, sizeof(struct tt__OnvifVersion));

        if(tds__GetServices->IncludeCapability == xsd__boolean__true_)
        {
            tService[i].Capabilities = (struct _tds__Service_Capabilities *)soap_malloc(soap, sizeof(struct _tds__Service_Capabilities));
            tService[i].Capabilities->__any = NULL;
        }
    }

    sprintf(IPAddr, "http://%s:%d/onvif/", inet_ntoa(*((struct in_addr *)&localIp)), ONVIF_SERVICE_PORT);
    printf("[%d] IPAddr ==== %s\n", __LINE__, IPAddr);

    strcpy(tService[0].Namespace, "http://www.onvif.org/ver10/device/wsdl");
    strcpy(tService[0].XAddr, IPAddr);
    strcat(tService[0].XAddr, "device_service");
    tService[0].Version->Major = 2;
    tService[0].Version->Minor = 40;

    strcpy(tService[1].Namespace, "http://www.onvif.org/ver10/media/wsdl");
    strcpy(tService[1].XAddr, IPAddr);
    strcat(tService[1].XAddr, "media");
    tService[1].Version->Major = 2;
    tService[1].Version->Minor = 40;

    /*
    strcpy(tService[2].Namespace, "http://www.onvif.org/ver10/events/wsdl");
    strcpy(tService[2].XAddr, IPAddr);
    strcat(tService[2].XAddr, "events");
    tService[2].Version->Major = 2;
    tService[2].Version->Minor = 40;

    strcpy(tService[3].Namespace, "http://www.onvif.org/ver10/ptz/wsdl");
    strcpy(tService[3].XAddr, IPAddr);
    strcat(tService[3].XAddr, "ptz");
    tService[3].Version->Major = 2;
    tService[3].Version->Minor = 40;
    */

    tds__GetServicesResponse->__sizeService = ServiceSize;
    tds__GetServicesResponse->Service = tService;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

//SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap* soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap* soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
	printf("[%d] %s enter !\n", __LINE__,__func__);

    tds__GetDeviceInformationResponse->Manufacturer = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    tds__GetDeviceInformationResponse->Model = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    tds__GetDeviceInformationResponse->FirmwareVersion = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    tds__GetDeviceInformationResponse->SerialNumber = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    tds__GetDeviceInformationResponse->HardwareId = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);

    strcpy(tds__GetDeviceInformationResponse->Manufacturer, "mstar");
    strcpy(tds__GetDeviceInformationResponse->Model, "mstar-ipc-2875");
    strcpy(tds__GetDeviceInformationResponse->FirmwareVersion, "mstar-ipc-001");
    strcpy(tds__GetDeviceInformationResponse->SerialNumber, "001-101-0j101");
    strcpy(tds__GetDeviceInformationResponse->HardwareId, "88888");

    printf("[%d] %s exit !\n", __LINE__,__func__);
    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap* soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap* soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
    struct tt__SystemDateTime *tSystemDateAndTime;
    onvif_system_date_time tSysDataTime;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetSysDataTime(&tSysDataTime);

    printf("[%s] timezone: %s \n",__func__,tSysDataTime.tTimeZone);
    printf("[%s] utc time: %d-%d-%d %d:%d:%d \n",__func__,
        tSysDataTime.tUtcTime.year,
        tSysDataTime.tUtcTime.month,
        tSysDataTime.tUtcTime.day,
        tSysDataTime.tUtcTime.hour,
        tSysDataTime.tUtcTime.minute,
        tSysDataTime.tUtcTime.second);

    printf("[%s] local time: %d-%d-%d %d:%d:%d \n",__func__,
        tSysDataTime.tLocalTime.year,
        tSysDataTime.tLocalTime.month,
        tSysDataTime.tLocalTime.day,
        tSysDataTime.tLocalTime.hour,
        tSysDataTime.tLocalTime.minute,
        tSysDataTime.tLocalTime.second);

    tSystemDateAndTime = (struct tt__SystemDateTime*)soap_malloc(soap, sizeof(struct tt__SystemDateTime));
    soap_default_tt__SystemDateTime(soap,tSystemDateAndTime);

    tSystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
    tSystemDateAndTime->DaylightSavings = xsd__boolean__false_;

    tSystemDateAndTime->TimeZone = (struct tt__TimeZone*)soap_malloc(soap, sizeof(struct tt__TimeZone));
    tSystemDateAndTime->TimeZone->TZ = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    strcpy(tSystemDateAndTime->TimeZone->TZ, tSysDataTime.tTimeZone);

    // utc data time
    tSystemDateAndTime->UTCDateTime = (struct tt__DateTime*)soap_malloc(soap, sizeof(struct tt__DateTime));
    tSystemDateAndTime->UTCDateTime->Time = (struct tt__Time*)soap_malloc(soap, sizeof(struct tt__Time));
    tSystemDateAndTime->UTCDateTime->Date = (struct tt__Date*)soap_malloc(soap, sizeof(struct tt__Date));

    tSystemDateAndTime->UTCDateTime->Time->Hour = tSysDataTime.tUtcTime.hour;
    tSystemDateAndTime->UTCDateTime->Time->Minute = tSysDataTime.tUtcTime.minute;
    tSystemDateAndTime->UTCDateTime->Time->Second = tSysDataTime.tUtcTime.second;

    tSystemDateAndTime->UTCDateTime->Date->Year = tSysDataTime.tUtcTime.year;
    tSystemDateAndTime->UTCDateTime->Date->Month = tSysDataTime.tUtcTime.month;
    tSystemDateAndTime->UTCDateTime->Date->Day = tSysDataTime.tUtcTime.day;

    // local data time
    tSystemDateAndTime->LocalDateTime = (struct tt__DateTime*)soap_malloc(soap, sizeof(struct tt__DateTime));
    tSystemDateAndTime->LocalDateTime->Time = (struct tt__Time*)soap_malloc(soap, sizeof(struct tt__Time));
    tSystemDateAndTime->LocalDateTime->Date = (struct tt__Date*)soap_malloc(soap, sizeof(struct tt__Date));

    tSystemDateAndTime->LocalDateTime->Time->Hour = tSysDataTime.tLocalTime.hour;
    tSystemDateAndTime->LocalDateTime->Time->Minute = tSysDataTime.tLocalTime.minute;
    tSystemDateAndTime->LocalDateTime->Time->Second = tSysDataTime.tLocalTime.second;

    tSystemDateAndTime->LocalDateTime->Date->Year = tSysDataTime.tLocalTime.year;
    tSystemDateAndTime->LocalDateTime->Date->Month = tSysDataTime.tLocalTime.month;
    tSystemDateAndTime->LocalDateTime->Date->Day = tSysDataTime.tLocalTime.day;

    tds__GetSystemDateAndTimeResponse->SystemDateAndTime = tSystemDateAndTime;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap* soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap* soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap* soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap* soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap* soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap* soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap* soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap* soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	int ScopesSize = 7;
    struct tt__Scope* tScope;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    tScope = (struct tt__Scope*)soap_malloc(soap, sizeof(struct tt__Scope) * ScopesSize);

    tScope[0].ScopeDef = tt__ScopeDefinition__Fixed;
    tScope[0].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[0].ScopeItem, "onvif://www.onvif.org/type/NetworkVideoTransmitter");

    tScope[1].ScopeDef = tt__ScopeDefinition__Fixed;
    tScope[1].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[1].ScopeItem, "onvif://www.onvif.org/type/video_encoder");

    tScope[2].ScopeDef = tt__ScopeDefinition__Fixed;
    tScope[2].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[2].ScopeItem, "onvif://www.onvif.org/type/audio_encoder");

    tScope[3].ScopeDef = tt__ScopeDefinition__Fixed;
    tScope[3].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[3].ScopeItem, "onvif://www.onvif.org/type/ptz");

    tScope[4].ScopeDef = tt__ScopeDefinition__Fixed;
    tScope[4].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[4].ScopeItem, "onvif://www.onvif.org/hardware/mstar-ipc-2875");

    tScope[5].ScopeDef = tt__ScopeDefinition__Configurable;
    tScope[5].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[5].ScopeItem, "onvif://www.onvif.org/name/Mstar IPC");//strcpy(tScope[5].ScopeItem, "onvif://www.onvif.org/name/mstar%20mstar-ipc-2875");

    tScope[6].ScopeDef = tt__ScopeDefinition__Configurable;
    tScope[6].ScopeItem = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tScope[6].ScopeItem, "onvif://www.onvif.org/location/city/shenzhen");

    tds__GetScopesResponse->__sizeScopes = ScopesSize;
    tds__GetScopesResponse->Scopes = tScope;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap* soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap* soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap* soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap* soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap* soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap* soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap* soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap* soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap* soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap* soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap* soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap* soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	int UserSize = 1;
    struct tt__User *tUser;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    tUser = (struct tt__User*)soap_malloc(soap, sizeof(struct tt__User) * UserSize);
    tUser[0].Username = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tUser[0].Username, "admin");

    tUser[0].UserLevel = tt__UserLevel__Administrator;
    tUser[0].Password = NULL;
    tUser[0].Extension = NULL;
    tUser[0].__anyAttribute = NULL;

    tds__GetUsersResponse->__sizeUser = UserSize;
    tds__GetUsersResponse->User = tUser;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap* soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap* soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap* soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap* soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap* soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	unsigned int localIp = 0;
    char IPAddr[COMMN_INFO_LEN];
    struct tt__Capabilities *tCapabilities;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetV4Ip(ETH_NAME, &localIp);
    sprintf(IPAddr, "http://%s:%d/onvif/", inet_ntoa(*((struct in_addr *)&localIp)), ONVIF_SERVICE_PORT);

    tCapabilities = (struct tt__Capabilities*)soap_malloc(soap, sizeof(struct tt__Capabilities));
    soap_default_tt__Capabilities(soap, tCapabilities);

    //device param
    tCapabilities->Device = (struct tt__DeviceCapabilities*)soap_malloc(soap, sizeof(struct tt__DeviceCapabilities));
    soap_default_tt__DeviceCapabilities(soap,tCapabilities->Device);

    tCapabilities->Device->XAddr = (char *) soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tCapabilities->Device->XAddr, IPAddr);
    strcat(tCapabilities->Device->XAddr, "device_service");

    //media param
    tCapabilities->Media = (struct tt__MediaCapabilities*)soap_malloc(soap, sizeof(struct tt__MediaCapabilities));
    soap_default_tt__MediaCapabilities(soap,tCapabilities->Media);

    tCapabilities->Media->XAddr = (char *) soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
    strcpy(tCapabilities->Media->XAddr, IPAddr);
    strcat(tCapabilities->Device->XAddr, "media");

    tCapabilities->Media->StreamingCapabilities = (struct tt__RealTimeStreamingCapabilities*)soap_malloc(soap, sizeof(struct tt__RealTimeStreamingCapabilities));
    soap_default_tt__RealTimeStreamingCapabilities(soap,tCapabilities->Media->StreamingCapabilities);

    tCapabilities->Media->StreamingCapabilities->RTPMulticast  = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    tCapabilities->Media->StreamingCapabilities->RTP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    tCapabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));

    *tCapabilities->Media->StreamingCapabilities->RTPMulticast  = xsd__boolean__false_;
    *tCapabilities->Media->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
    *tCapabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;

    //extension param
    tCapabilities->Extension = (struct tt__CapabilitiesExtension*)soap_malloc(soap, sizeof(struct tt__CapabilitiesExtension));
    soap_default_tt__CapabilitiesExtension(soap,tCapabilities->Extension);
    tCapabilities->Extension->DeviceIO = (struct tt__DeviceIOCapabilities*)soap_malloc(soap, sizeof(struct tt__DeviceIOCapabilities));
    soap_default_tt__DeviceIOCapabilities(soap,tCapabilities->Extension->DeviceIO);
	tCapabilities->Extension->DeviceIO->VideoSources = 1;

    tds__GetCapabilitiesResponse->Capabilities = tCapabilities;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap* soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap* soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	char hostName[SMALL_INFO_LEN];
	struct tt__HostnameInformation *tHostname;

	printf("[%d] %s enter !\n", __LINE__,__func__);

	gethostname(hostName,SMALL_INFO_LEN);
	printf("hostname:%s\n",hostName);

	tHostname = (struct tt__HostnameInformation*)soap_malloc(soap, sizeof(struct tt__HostnameInformation));

	tHostname->FromDHCP = xsd__boolean__false_;
	tHostname->Name = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
	strcpy(tHostname->Name, hostName);
	tHostname->Extension = NULL;
	tHostname->__anyAttribute = NULL;

	tds__GetHostnameResponse->HostnameInformation = tHostname;

	printf("[%d] %s exit !\n", __LINE__,__func__);

	return SOAP_OK;
}
#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap* soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap* soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap* soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	int i = 0;
    struct tt__DNSInformation *tDNSInformation;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    tDNSInformation = (struct tt__DNSInformation*)soap_malloc(soap, sizeof(struct tt__DNSInformation));
    soap_default_tt__DNSInformation(soap,tDNSInformation);

    tDNSInformation->FromDHCP = xsd__boolean__false_;

    tDNSInformation->__sizeDNSManual = 1;
    tDNSInformation->DNSManual = (struct tt__IPAddress*)soap_malloc(soap, sizeof(struct tt__IPAddress)*tDNSInformation->__sizeDNSManual);

    for(i = 0;i < tDNSInformation->__sizeDNSManual;i++)
    {
        tDNSInformation->DNSManual[i].Type = tt__IPType__IPv4;
        tDNSInformation->DNSManual[i].IPv6Address = NULL;
        tDNSInformation->DNSManual[i].IPv4Address = (char *)soap_malloc(soap, sizeof(char) * COMMN_INFO_LEN);
        strcpy(tDNSInformation->DNSManual[i].IPv4Address, "8.8.8.8");

        printf("[%d] DNS Addr ==== %s\n", __LINE__, tDNSInformation->DNSManual[i].IPv4Address);
    }

    tds__GetDNSResponse->DNSInformation = tDNSInformation;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap* soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap* soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	struct tt__NTPInformation *tNTPInformation;

	printf("[%d] %s enter !\n", __LINE__,__func__);

	tNTPInformation = (struct tt__NTPInformation*)soap_malloc(soap, sizeof(struct tt__NTPInformation));
	soap_default_tt__NTPInformation(soap,tNTPInformation);

	tNTPInformation->FromDHCP = xsd__boolean__false_;
	tNTPInformation->__sizeNTPManual = 1;

	tNTPInformation->NTPManual = (struct tt__NetworkHost*)soap_malloc(soap, sizeof(struct tt__NetworkHost));
	soap_default_tt__NetworkHost(soap,tNTPInformation->NTPManual);
	tNTPInformation->NTPManual->Type = tt__NetworkHostType__DNS;

	tNTPInformation->NTPManual->DNSname = (char *) soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
	strcpy(tNTPInformation->NTPManual->DNSname, "time.windows.com");

	printf("[%d] %s exit !\n", __LINE__,__func__);

	tds__GetNTPResponse->NTPInformation = tNTPInformation;

	return SOAP_OK;
}

#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap* soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap* soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap* soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap* soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	int i = 0;
	int NetworkNum = 1; //this will deside by the real network
	unsigned int  V4Addr = 0;
	char V6Addr[80];
	unsigned char MacAddr[6];
	struct tt__NetworkInterface* tNetworkInterface;

	printf("[%d] %s enter !\n", __LINE__,__func__);

	mst_GetMac(ETH_NAME, MacAddr);
	mst_GetV4Ip(ETH_NAME, &V4Addr);
	mst_GetV6Ip(ETH_NAME, V6Addr);

	tNetworkInterface = (struct tt__NetworkInterface*)soap_malloc(soap, sizeof(struct tt__NetworkInterface) * NetworkNum);

	for(i = 0;i < NetworkNum;i++)
	{
		soap_default_tt__NetworkInterface(soap,&tNetworkInterface[i]);

		tNetworkInterface[i].token = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		strcpy(tNetworkInterface[i].token, ETH_NAME);

		tNetworkInterface[i].Enabled = xsd__boolean__true_;

		//info
		tNetworkInterface[i].Info = (struct tt__NetworkInterfaceInfo*)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceInfo));
		tNetworkInterface[i].Info->Name = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		tNetworkInterface[i].Info->HwAddress = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		tNetworkInterface[i].Info->MTU = (int *)soap_malloc(soap, sizeof(int));

		strcpy(tNetworkInterface[i].Info->Name, ETH_NAME);
		sprintf(tNetworkInterface[i].Info->HwAddress,"%02x:%02x:%02x:%02x:%02x:%02x",MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
		*tNetworkInterface[i].Info->MTU = 1500;

		//link
		tNetworkInterface[i].Link = (struct tt__NetworkInterfaceLink*)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceLink));
		tNetworkInterface[i].Link->AdminSettings = (struct tt__NetworkInterfaceConnectionSetting*)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
		tNetworkInterface[i].Link->AdminSettings->AutoNegotiation = xsd__boolean__true_;
		tNetworkInterface[i].Link->AdminSettings->Duplex = tt__Duplex__Full;
		tNetworkInterface[i].Link->AdminSettings->Speed = 100;

		tNetworkInterface[i].Link->OperSettings = (struct tt__NetworkInterfaceConnectionSetting*)soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
		tNetworkInterface[i].Link->OperSettings->AutoNegotiation = xsd__boolean__true_;
		tNetworkInterface[i].Link->OperSettings->Duplex = tt__Duplex__Full;
		tNetworkInterface[i].Link->OperSettings->Speed = 100;

		tNetworkInterface[i].Link->InterfaceType = 0;

		//IpV4
		tNetworkInterface[i].IPv4 = (struct tt__IPv4NetworkInterface*)soap_malloc(soap, sizeof(struct tt__IPv4NetworkInterface));
		tNetworkInterface[i].IPv4->Enabled = xsd__boolean__true_;

		tNetworkInterface[i].IPv4->Config = (struct tt__IPv4Configuration*)soap_malloc(soap, sizeof(struct tt__IPv4Configuration));
		soap_default_tt__IPv4Configuration(soap,tNetworkInterface[i].IPv4->Config);
		tNetworkInterface[i].IPv4->Config->__sizeManual = 1;

		tNetworkInterface[i].IPv4->Config->Manual = (struct tt__PrefixedIPv4Address*)soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
		tNetworkInterface[i].IPv4->Config->Manual->Address = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		sprintf(tNetworkInterface[i].IPv4->Config->Manual->Address, "%s", inet_ntoa(*((struct in_addr *)&V4Addr)));
		tNetworkInterface[i].IPv4->Config->Manual->PrefixLength = 24;

		//IpV6
		tNetworkInterface[i].IPv6 = (struct tt__IPv6NetworkInterface*)soap_malloc(soap, sizeof(struct tt__IPv6NetworkInterface));
		tNetworkInterface[i].IPv6->Enabled = xsd__boolean__true_;

		tNetworkInterface[i].IPv6->Config = (struct tt__IPv6Configuration*)soap_malloc(soap, sizeof(struct tt__IPv6Configuration));
		soap_default_tt__IPv6Configuration(soap,tNetworkInterface[i].IPv6->Config);

		tNetworkInterface[i].IPv6->Config->AcceptRouterAdvert = (enum xsd__boolean*)soap_malloc(soap, sizeof(enum xsd__boolean));
		*tNetworkInterface[i].IPv6->Config->AcceptRouterAdvert = xsd__boolean__false_;
		tNetworkInterface[i].IPv6->Config->DHCP = tt__IPv6DHCPConfiguration__Off;

		tNetworkInterface[i].IPv6->Config->__sizeLinkLocal = 1;
		tNetworkInterface[i].IPv6->Config->LinkLocal = (struct tt__PrefixedIPv6Address*)soap_malloc(soap, sizeof(struct tt__PrefixedIPv6Address));
		tNetworkInterface[i].IPv6->Config->LinkLocal->Address = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		strcpy(tNetworkInterface[i].IPv6->Config->LinkLocal->Address, V6Addr);
		tNetworkInterface[i].IPv6->Config->LinkLocal->PrefixLength = 64;

		tNetworkInterface[i].IPv6->Config->__sizeFromDHCP = 1;
		tNetworkInterface[i].IPv6->Config->FromDHCP = (struct tt__PrefixedIPv6Address*)soap_malloc(soap, sizeof(struct tt__PrefixedIPv6Address));
		tNetworkInterface[i].IPv6->Config->FromDHCP->Address = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		strcpy(tNetworkInterface[i].IPv6->Config->FromDHCP->Address, V6Addr);
		tNetworkInterface[i].IPv6->Config->FromDHCP->PrefixLength = 64;

	}

	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = NetworkNum;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces = tNetworkInterface;

	printf("[%d] %s exit !\n", __LINE__,__func__);

	return SOAP_OK;
}

#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap* soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap* soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap* soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap* soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	char DefaultGateway[SMALL_INFO_LEN];
	struct tt__NetworkGateway *tNetworkGateway = NULL;

	printf("[%d] %s enter !\n", __LINE__,__func__);

	if(mst_GetDefaultGateway(ETH_NAME,DefaultGateway) == 0)
	{
		printf("[%d] %s DefaultGateway=%s !\n", __LINE__,__func__,DefaultGateway);

		tNetworkGateway = (struct tt__NetworkGateway*)soap_malloc(soap, sizeof(struct tt__NetworkGateway));
		tNetworkGateway->__sizeIPv4Address = 1;
		tNetworkGateway->IPv4Address = (char **)soap_malloc(soap,sizeof(char*));
		tNetworkGateway->IPv4Address[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		strcpy(tNetworkGateway->IPv4Address[0], DefaultGateway);
		//tNetworkGateway->__sizeIPv6Address = 1;
		tNetworkGateway->IPv6Address = NULL;//(char **)soap_malloc(soap,sizeof(char*));
		//tNetworkGateway->IPv6Address[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
		//strcpy(tNetworkGateway->IPv6Address[0], DefaultGateway);
	}
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway = tNetworkGateway;

	printf("[%d] %s exit !\n", __LINE__,__func__);

	return SOAP_OK;
}

//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap* soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap* soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	unsigned int  V4Addr = 0;
    struct tt__NetworkZeroConfiguration *tZeroConfiguration;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetV4Ip(ETH_NAME, &V4Addr);

    tZeroConfiguration = (struct tt__NetworkZeroConfiguration*)soap_malloc(soap, sizeof(struct tt__NetworkZeroConfiguration));

    tZeroConfiguration->InterfaceToken = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    strcpy(tZeroConfiguration->InterfaceToken, ETH_NAME);

    tZeroConfiguration->Enabled = xsd__boolean__true_;
    tZeroConfiguration->__sizeAddresses = 1;
    tZeroConfiguration->Addresses = (char **)soap_malloc(soap,sizeof(char*));
    tZeroConfiguration->Addresses[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LEN);
    sprintf(tZeroConfiguration->Addresses[0], "%s", inet_ntoa(*((struct in_addr *)&V4Addr)));

    tZeroConfiguration->Extension = NULL;
    tZeroConfiguration->__anyAttribute = NULL;

    tds__GetZeroConfigurationResponse->ZeroConfiguration = tZeroConfiguration;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
#if 0 //struct not declard
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap* soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap* soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap* soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap* soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap* soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap* soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap* soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap* soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap* soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap* soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap* soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap* soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap* soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap* soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap* soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap* soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap* soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap* soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap* soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap* soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap* soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap* soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap* soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap* soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap* soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap* soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap* soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap* soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap* soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap* soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap* soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap* soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap* soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages(struct soap* soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Seek(struct soap* soap, struct _tev__Seek *tev__Seek, struct _tev__SeekResponse *tev__SeekResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__SetSynchronizationPoint(struct soap* soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetServiceCapabilities(struct soap* soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription(struct soap* soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventProperties(struct soap* soap, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew(struct soap* soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe(struct soap* soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe(struct soap* soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetCurrentMessage(struct soap* soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap* soap, struct _wsnt__Notify *wsnt__Notify){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages(struct soap* soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint(struct soap* soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify_(struct soap* soap, struct _wsnt__Notify *wsnt__Notify){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint(struct soap* soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew_(struct soap* soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe_(struct soap* soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PauseSubscription(struct soap* soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tev__ResumeSubscription(struct soap* soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap* soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap* soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	float _Brightness;
	float _Saturation;
	float _Contrast;
	float _Sharpness;
	//float _WideDynamicRange;
	int _Backlight;
	float _BacklightCompensationLevel;
	//int _WhiteBalance;
	enum tt__IrCutFilterMode eIrCutFilter;

	_Brightness = (float)(50);
	_Saturation = (float)(50);
	_Contrast = (float)(50);
	_Sharpness = (float)(50);
	//_WideDynamicRange = (float)(50);
	_Backlight = 50;
	_BacklightCompensationLevel = 50;
	//_WhiteBalance = 50;
	eIrCutFilter = tt__IrCutFilterMode__AUTO;

	timg__GetImagingSettingsResponse->ImagingSettings = (struct tt__ImagingSettings20 *)soap_malloc(soap, sizeof(struct tt__ImagingSettings20));
	timg__GetImagingSettingsResponse->ImagingSettings->Brightness = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness = _Brightness;
	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = _Saturation;
	timg__GetImagingSettingsResponse->ImagingSettings->Contrast = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast = _Contrast;
	timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = (enum tt__IrCutFilterMode *)soap_malloc(soap, sizeof(int));
	*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = eIrCutFilter;
	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = (float *)soap_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = _Sharpness;
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation = (struct tt__BacklightCompensation20 *)soap_malloc(soap, sizeof(struct tt__BacklightCompensation20));
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = _Backlight;
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level =(float *) soap_malloc(soap, sizeof(float));//float
	*timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Level =_BacklightCompensationLevel;//float
	timg__GetImagingSettingsResponse->ImagingSettings->Exposure = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->Focus = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange = NULL ;
	timg__GetImagingSettingsResponse->ImagingSettings->WhiteBalance  = NULL ;
	timg__GetImagingSettingsResponse->ImagingSettings->Extension = NULL;
	timg__GetImagingSettingsResponse->ImagingSettings->__anyAttribute =NULL;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap* soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
#if 0
	char cBrightnessFlag = -1;
	char cSaturationFlag = -1;
	char cContrastFlag = -1;
	char cSharpnessFlag =-1;
	char cMIrisFlag=-1;
	char cFocusFlag=-1;
	if(timg__SetImagingSettings->ImagingSettings->Brightness != NULL)
	{
		cBrightnessFlag = 1;
	}
	if(timg__SetImagingSettings->ImagingSettings->ColorSaturation != NULL)
	{
		cSaturationFlag =1;
	}
	if(timg__SetImagingSettings->ImagingSettings->Contrast != NULL)
	{
		cContrastFlag = 1;
	}
	if(timg__SetImagingSettings->ImagingSettings->Sharpness != NULL)
	{

	}
	if(timg__SetImagingSettings->ImagingSettings->Exposure != NULL)
	{
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Gain != NULL)
		{

		}
		if(timg__SetImagingSettings->ImagingSettings->Exposure->Iris != NULL)
		{
			float  fIris  =*timg__SetImagingSettings->ImagingSettings->Exposure->Iris;
			if(fIris > 0)
			{
				cMIrisFlag = 1;
			}
			else if(fIris <0 )
			{
				cMIrisFlag = 0;
			}
			else
			{
				cMIrisFlag = 0xFF;
			}
		}
	}
	if(timg__SetImagingSettings->ImagingSettings->Focus!= NULL)
	{
		if(timg__SetImagingSettings->ImagingSettings->Focus->FarLimit !=NULL)
		{
			if(timg__SetImagingSettings->ImagingSettings->Focus->FarLimit >0 )
			{
				cFocusFlag = 1;
			}
			else
			{
				cFocusFlag = 0;
			}
		}
		if(timg__SetImagingSettings->ImagingSettings->Focus->NearLimit !=NULL)
		{
			if(timg__SetImagingSettings->ImagingSettings->Focus->NearLimit >0 )
			{
				cFocusFlag = 0;
			}
			else
			{
				cFocusFlag = 1;
			}
		}
		if(timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed!=NULL)
		{
			if(timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed >0)
			{
				cFocusFlag = 1;
			}
			else
			{
				cFocusFlag = 0;
			}
		}
	}
#endif
	return SOAP_OK;
}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap* soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap* soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap* soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap* soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap* soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetServiceCapabilities(struct soap* soap, struct _tls__GetServiceCapabilities *tls__GetServiceCapabilities, struct _tls__GetServiceCapabilitiesResponse *tls__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetLayout(struct soap* soap, struct _tls__GetLayout *tls__GetLayout, struct _tls__GetLayoutResponse *tls__GetLayoutResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetLayout(struct soap* soap, struct _tls__SetLayout *tls__SetLayout, struct _tls__SetLayoutResponse *tls__SetLayoutResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetDisplayOptions(struct soap* soap, struct _tls__GetDisplayOptions *tls__GetDisplayOptions, struct _tls__GetDisplayOptionsResponse *tls__GetDisplayOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetPaneConfigurations(struct soap* soap, struct _tls__GetPaneConfigurations *tls__GetPaneConfigurations, struct _tls__GetPaneConfigurationsResponse *tls__GetPaneConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__GetPaneConfiguration(struct soap* soap, struct _tls__GetPaneConfiguration *tls__GetPaneConfiguration, struct _tls__GetPaneConfigurationResponse *tls__GetPaneConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetPaneConfigurations(struct soap* soap, struct _tls__SetPaneConfigurations *tls__SetPaneConfigurations, struct _tls__SetPaneConfigurationsResponse *tls__SetPaneConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__SetPaneConfiguration(struct soap* soap, struct _tls__SetPaneConfiguration *tls__SetPaneConfiguration, struct _tls__SetPaneConfigurationResponse *tls__SetPaneConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__CreatePaneConfiguration(struct soap* soap, struct _tls__CreatePaneConfiguration *tls__CreatePaneConfiguration, struct _tls__CreatePaneConfigurationResponse *tls__CreatePaneConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tls__DeletePaneConfiguration(struct soap* soap, struct _tls__DeletePaneConfiguration *tls__DeletePaneConfiguration, struct _tls__DeletePaneConfigurationResponse *tls__DeletePaneConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetServiceCapabilities(struct soap* soap, struct _tmd__GetServiceCapabilities *tmd__GetServiceCapabilities, struct _tmd__GetServiceCapabilitiesResponse *tmd__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputOptions(struct soap* soap, struct _tmd__GetRelayOutputOptions *tmd__GetRelayOutputOptions, struct _tmd__GetRelayOutputOptionsResponse *tmd__GetRelayOutputOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSources(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputs(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	struct tt__VideoSource *tVideoSource;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    tVideoSource = (struct tt__VideoSource*)soap_malloc(soap, sizeof(struct tt__VideoSource));
    soap_default_tt__VideoSource(soap,tVideoSource);

    tVideoSource->token = (char *)soap_malloc(soap, sizeof(char)*COMMN_INFO_LEN);
    strcpy(tVideoSource->token,"VideoSource_1"); //same to getProfiles

    tVideoSource->Framerate = 30;

    tVideoSource->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    tVideoSource->Resolution->Width = 1920;
    tVideoSource->Resolution->Height = 1080;

    tVideoSource->Imaging =(struct tt__ImagingSettings*)soap_malloc(soap, sizeof(struct tt__ImagingSettings));
    soap_default_tt__ImagingSettings(soap,tVideoSource->Imaging);

    tVideoSource->Imaging->BacklightCompensation = (struct tt__BacklightCompensation*)soap_malloc(soap, sizeof(struct tt__BacklightCompensation));
    tVideoSource->Imaging->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
    tVideoSource->Imaging->BacklightCompensation->Level = 0;

    tVideoSource->Imaging->Brightness = (float*)soap_malloc(soap,sizeof(float));
    *tVideoSource->Imaging->Brightness = 128;

    tVideoSource->Imaging->ColorSaturation = (float*)soap_malloc(soap,sizeof(float));
    *tVideoSource->Imaging->ColorSaturation = 128;

    tVideoSource->Imaging->Contrast = (float*)soap_malloc(soap,sizeof(float));
    *tVideoSource->Imaging->Contrast = 128;

    tVideoSource->Imaging->IrCutFilter = (enum tt__IrCutFilterMode *)soap_malloc(soap,sizeof(enum tt__IrCutFilterMode));
    *tVideoSource->Imaging->IrCutFilter = tt__IrCutFilterMode__AUTO;

    tVideoSource->Imaging->Sharpness = (float*)soap_malloc(soap,sizeof(float));
    *tVideoSource->Imaging->Sharpness = 128;

    tVideoSource->Imaging->WideDynamicRange = (struct tt__WideDynamicRange*)soap_malloc(soap, sizeof(struct tt__WideDynamicRange));
    tVideoSource->Imaging->WideDynamicRange->Mode = tt__WideDynamicMode__OFF;
    tVideoSource->Imaging->WideDynamicRange->Level = 0;

    tVideoSource->Imaging->WhiteBalance = (struct tt__WhiteBalance*)soap_malloc(soap, sizeof(struct tt__WhiteBalance));
    soap_default_tt__WhiteBalance(soap,tVideoSource->Imaging->WhiteBalance);

    tVideoSource->Imaging->WhiteBalance->Mode = tt__WhiteBalanceMode__AUTO;
    tVideoSource->Imaging->WhiteBalance->CrGain = 0;
    tVideoSource->Imaging->WhiteBalance->CbGain = 0;

    trt__GetVideoSourcesResponse->__sizeVideoSources = 1;
    trt__GetVideoSourcesResponse->VideoSources = tVideoSource;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputs(struct soap* soap, struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs, struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfiguration(struct soap* soap, struct _tmd__GetVideoSourceConfiguration *tmd__GetVideoSourceConfiguration, struct _tmd__GetVideoSourceConfigurationResponse *tmd__GetVideoSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfiguration(struct soap* soap, struct _tmd__GetVideoOutputConfiguration *tmd__GetVideoOutputConfiguration, struct _tmd__GetVideoOutputConfigurationResponse *tmd__GetVideoOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfiguration(struct soap* soap, struct _tmd__GetAudioSourceConfiguration *tmd__GetAudioSourceConfiguration, struct _tmd__GetAudioSourceConfigurationResponse *tmd__GetAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfiguration(struct soap* soap, struct _tmd__GetAudioOutputConfiguration *tmd__GetAudioOutputConfiguration, struct _tmd__GetAudioOutputConfigurationResponse *tmd__GetAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoSourceConfiguration(struct soap* soap, struct _tmd__SetVideoSourceConfiguration *tmd__SetVideoSourceConfiguration, struct _tmd__SetVideoSourceConfigurationResponse *tmd__SetVideoSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoOutputConfiguration(struct soap* soap, struct _tmd__SetVideoOutputConfiguration *tmd__SetVideoOutputConfiguration, struct _tmd__SetVideoOutputConfigurationResponse *tmd__SetVideoOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioSourceConfiguration(struct soap* soap, struct _tmd__SetAudioSourceConfiguration *tmd__SetAudioSourceConfiguration, struct _tmd__SetAudioSourceConfigurationResponse *tmd__SetAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioOutputConfiguration(struct soap* soap, struct _tmd__SetAudioOutputConfiguration *tmd__SetAudioOutputConfiguration, struct _tmd__SetAudioOutputConfigurationResponse *tmd__SetAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfigurationOptions(struct soap* soap, struct _tmd__GetVideoSourceConfigurationOptions *tmd__GetVideoSourceConfigurationOptions, struct _tmd__GetVideoSourceConfigurationOptionsResponse *tmd__GetVideoSourceConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfigurationOptions(struct soap* soap, struct _tmd__GetVideoOutputConfigurationOptions *tmd__GetVideoOutputConfigurationOptions, struct _tmd__GetVideoOutputConfigurationOptionsResponse *tmd__GetVideoOutputConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfigurationOptions(struct soap* soap, struct _tmd__GetAudioSourceConfigurationOptions *tmd__GetAudioSourceConfigurationOptions, struct _tmd__GetAudioSourceConfigurationOptionsResponse *tmd__GetAudioSourceConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfigurationOptions(struct soap* soap, struct _tmd__GetAudioOutputConfigurationOptions *tmd__GetAudioOutputConfigurationOptions, struct _tmd__GetAudioOutputConfigurationOptionsResponse *tmd__GetAudioOutputConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputSettings(struct soap* soap, struct _tmd__SetRelayOutputSettings *tmd__SetRelayOutputSettings, struct _tmd__SetRelayOutputSettingsResponse *tmd__SetRelayOutputSettingsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputState(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetDigitalInputs(struct soap* soap, struct _tmd__GetDigitalInputs *tmd__GetDigitalInputs, struct _tmd__GetDigitalInputsResponse *tmd__GetDigitalInputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPorts(struct soap* soap, struct _tmd__GetSerialPorts *tmd__GetSerialPorts, struct _tmd__GetSerialPortsResponse *tmd__GetSerialPortsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfiguration(struct soap* soap, struct _tmd__GetSerialPortConfiguration *tmd__GetSerialPortConfiguration, struct _tmd__GetSerialPortConfigurationResponse *tmd__GetSerialPortConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetSerialPortConfiguration(struct soap* soap, struct _tmd__SetSerialPortConfiguration *tmd__SetSerialPortConfiguration, struct _tmd__SetSerialPortConfigurationResponse *tmd__SetSerialPortConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfigurationOptions(struct soap* soap, struct _tmd__GetSerialPortConfigurationOptions *tmd__GetSerialPortConfigurationOptions, struct _tmd__GetSerialPortConfigurationOptionsResponse *tmd__GetSerialPortConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SendReceiveSerialCommand(struct soap* soap, struct _tmd__SendReceiveSerialCommand *tmd__SendReceiveSerialCommand, struct _tmd__SendReceiveSerialCommandResponse *tmd__SendReceiveSerialCommandResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap* soap, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap* soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap* soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap* soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap* soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap* soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap* soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap* soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap* soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap* soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap* soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap* soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap* soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap* soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap* soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap* soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap* soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap* soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap* soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTours(struct soap* soap, struct _tptz__GetPresetTours *tptz__GetPresetTours, struct _tptz__GetPresetToursResponse *tptz__GetPresetToursResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTour(struct soap* soap, struct _tptz__GetPresetTour *tptz__GetPresetTour, struct _tptz__GetPresetTourResponse *tptz__GetPresetTourResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTourOptions(struct soap* soap, struct _tptz__GetPresetTourOptions *tptz__GetPresetTourOptions, struct _tptz__GetPresetTourOptionsResponse *tptz__GetPresetTourOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__CreatePresetTour(struct soap* soap, struct _tptz__CreatePresetTour *tptz__CreatePresetTour, struct _tptz__CreatePresetTourResponse *tptz__CreatePresetTourResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ModifyPresetTour(struct soap* soap, struct _tptz__ModifyPresetTour *tptz__ModifyPresetTour, struct _tptz__ModifyPresetTourResponse *tptz__ModifyPresetTourResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__OperatePresetTour(struct soap* soap, struct _tptz__OperatePresetTour *tptz__OperatePresetTour, struct _tptz__OperatePresetTourResponse *tptz__OperatePresetTourResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePresetTour(struct soap* soap, struct _tptz__RemovePresetTour *tptz__RemovePresetTour, struct _tptz__RemovePresetTourResponse *tptz__RemovePresetTourResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetCompatibleConfigurations(struct soap* soap, struct _tptz__GetCompatibleConfigurations *tptz__GetCompatibleConfigurations, struct _tptz__GetCompatibleConfigurationsResponse *tptz__GetCompatibleConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetServiceCapabilities(struct soap* soap, struct _trc__GetServiceCapabilities *trc__GetServiceCapabilities, struct _trc__GetServiceCapabilitiesResponse *trc__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecording(struct soap* soap, struct _trc__CreateRecording *trc__CreateRecording, struct _trc__CreateRecordingResponse *trc__CreateRecordingResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecording(struct soap* soap, struct _trc__DeleteRecording *trc__DeleteRecording, struct _trc__DeleteRecordingResponse *trc__DeleteRecordingResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordings(struct soap* soap, struct _trc__GetRecordings *trc__GetRecordings, struct _trc__GetRecordingsResponse *trc__GetRecordingsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingConfiguration(struct soap* soap, struct _trc__SetRecordingConfiguration *trc__SetRecordingConfiguration, struct _trc__SetRecordingConfigurationResponse *trc__SetRecordingConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingConfiguration(struct soap* soap, struct _trc__GetRecordingConfiguration *trc__GetRecordingConfiguration, struct _trc__GetRecordingConfigurationResponse *trc__GetRecordingConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingOptions(struct soap* soap, struct _trc__GetRecordingOptions *trc__GetRecordingOptions, struct _trc__GetRecordingOptionsResponse *trc__GetRecordingOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateTrack(struct soap* soap, struct _trc__CreateTrack *trc__CreateTrack, struct _trc__CreateTrackResponse *trc__CreateTrackResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteTrack(struct soap* soap, struct _trc__DeleteTrack *trc__DeleteTrack, struct _trc__DeleteTrackResponse *trc__DeleteTrackResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetTrackConfiguration(struct soap* soap, struct _trc__GetTrackConfiguration *trc__GetTrackConfiguration, struct _trc__GetTrackConfigurationResponse *trc__GetTrackConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetTrackConfiguration(struct soap* soap, struct _trc__SetTrackConfiguration *trc__SetTrackConfiguration, struct _trc__SetTrackConfigurationResponse *trc__SetTrackConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__CreateRecordingJob(struct soap* soap, struct _trc__CreateRecordingJob *trc__CreateRecordingJob, struct _trc__CreateRecordingJobResponse *trc__CreateRecordingJobResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__DeleteRecordingJob(struct soap* soap, struct _trc__DeleteRecordingJob *trc__DeleteRecordingJob, struct _trc__DeleteRecordingJobResponse *trc__DeleteRecordingJobResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobs(struct soap* soap, struct _trc__GetRecordingJobs *trc__GetRecordingJobs, struct _trc__GetRecordingJobsResponse *trc__GetRecordingJobsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobConfiguration(struct soap* soap, struct _trc__SetRecordingJobConfiguration *trc__SetRecordingJobConfiguration, struct _trc__SetRecordingJobConfigurationResponse *trc__SetRecordingJobConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobConfiguration(struct soap* soap, struct _trc__GetRecordingJobConfiguration *trc__GetRecordingJobConfiguration, struct _trc__GetRecordingJobConfigurationResponse *trc__GetRecordingJobConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__SetRecordingJobMode(struct soap* soap, struct _trc__SetRecordingJobMode *trc__SetRecordingJobMode, struct _trc__SetRecordingJobModeResponse *trc__SetRecordingJobModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trc__GetRecordingJobState(struct soap* soap, struct _trc__GetRecordingJobState *trc__GetRecordingJobState, struct _trc__GetRecordingJobStateResponse *trc__GetRecordingJobStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetServiceCapabilities(struct soap* soap, struct _trp__GetServiceCapabilities *trp__GetServiceCapabilities, struct _trp__GetServiceCapabilitiesResponse *trp__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayUri(struct soap* soap, struct _trp__GetReplayUri *trp__GetReplayUri, struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayConfiguration(struct soap* soap, struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration, struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trp__SetReplayConfiguration(struct soap* soap, struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration, struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap* soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap* soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap* soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	struct tt__Profile *tProfile;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    // profile_1

    tProfile = (struct tt__Profile *)soap_malloc(soap,sizeof(struct tt__Profile));

    //main stream profiles
    soap_default_tt__Profile(soap,tProfile);

    tProfile->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->Name,"mainStream");

    tProfile->token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->token,"Profile_1");

    tProfile->fixed = (enum xsd__boolean *)soap_malloc(soap,sizeof(enum xsd__boolean));
    *tProfile->fixed = xsd__boolean__true_;

    // video source configuration
    tProfile->VideoSourceConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration ));
    soap_default_tt__VideoSourceConfiguration(soap,tProfile->VideoSourceConfiguration);

    tProfile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->VideoSourceConfiguration->Name,"VideoSourceConfig");

    tProfile->VideoSourceConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->VideoSourceConfiguration->token,"VideoSourceToken");

    tProfile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->VideoSourceConfiguration->SourceToken,"VideoSource_1"); // must same to __tmd__GetVideoSources's token

    tProfile->VideoSourceConfiguration->UseCount = 1;

    tProfile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap,sizeof(struct tt__IntRectangle));
    tProfile->VideoSourceConfiguration->Bounds->x = 0;
    tProfile->VideoSourceConfiguration->Bounds->y = 0;
    tProfile->VideoSourceConfiguration->Bounds->width = 1920;
    tProfile->VideoSourceConfiguration->Bounds->height = 1080;

    // audio source configuration

    tProfile->AudioSourceConfiguration = (struct tt__AudioSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__AudioSourceConfiguration ));
    soap_default_tt__AudioSourceConfiguration(soap,tProfile->AudioSourceConfiguration);

    tProfile->AudioSourceConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->AudioSourceConfiguration->Name,"AudioSourceConfig");

    tProfile->AudioSourceConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->AudioSourceConfiguration->token,"AudioSourceToken");

    tProfile->AudioSourceConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->AudioSourceConfiguration->SourceToken,"AudioSourceChannel1");

    tProfile->AudioSourceConfiguration->UseCount = 1;

    // video encoder configuration
    tProfile->VideoEncoderConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));
    soap_default_tt__VideoEncoderConfiguration(soap,tProfile->VideoEncoderConfiguration);

    tProfile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->VideoEncoderConfiguration->Name,"VideoEncoder_1");

    tProfile->VideoEncoderConfiguration->token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->VideoEncoderConfiguration->token,"VideoEncoder_Token");

    tProfile->VideoEncoderConfiguration->UseCount = 1;
    tProfile->VideoEncoderConfiguration->Quality = 3.0;
    tProfile->VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264; //JPEG = 0, MPEG4 = 1, H264 = 2;

    tProfile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    tProfile->VideoEncoderConfiguration->Resolution->Width = 1920;
    tProfile->VideoEncoderConfiguration->Resolution->Height = 1080;

    tProfile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    tProfile->VideoEncoderConfiguration->RateControl->FrameRateLimit = 30;
    tProfile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
    tProfile->VideoEncoderConfiguration->RateControl->BitrateLimit = 4096;

    // audio encoder configuration
    tProfile->AudioEncoderConfiguration = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__AudioEncoderConfiguration));
    soap_default_tt__AudioEncoderConfiguration(soap,tProfile->AudioEncoderConfiguration);

    tProfile->AudioEncoderConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->AudioEncoderConfiguration->Name,"AudioEncoder_1");

    tProfile->AudioEncoderConfiguration->token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile->AudioEncoderConfiguration->token,"AudioEncoder_Token");

    tProfile->AudioEncoderConfiguration->UseCount = 1;
    tProfile->AudioEncoderConfiguration->Encoding = tt__AudioEncoding__G711;

    tProfile->AudioEncoderConfiguration->Bitrate = 64;
    tProfile->AudioEncoderConfiguration->SampleRate = 8;

    //tProfile[0].AudioEncoderConfiguration->SessionTimeout = 3000;

    trt__GetProfileResponse->Profile = tProfile;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	int ProfileSize = 1;
    struct tt__Profile *tProfile;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    tProfile = (struct tt__Profile *)soap_malloc(soap,sizeof(struct tt__Profile)*ProfileSize);

    //main stream profiles
    soap_default_tt__Profile(soap,&tProfile[0]);

    tProfile[0].Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].Name,"mainStream");

    tProfile[0].token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].token,"Profile_1");

    tProfile[0].fixed = (enum xsd__boolean *)soap_malloc(soap,sizeof(enum xsd__boolean));
    *tProfile[0].fixed = xsd__boolean__true_;

    // video source configuration
    tProfile[0].VideoSourceConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration ));
    soap_default_tt__VideoSourceConfiguration(soap,tProfile[0].VideoSourceConfiguration);

    tProfile[0].VideoSourceConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].VideoSourceConfiguration->Name,"VideoSourceConfig");

    tProfile[0].VideoSourceConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].VideoSourceConfiguration->token,"VideoSourceToken");

    tProfile[0].VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].VideoSourceConfiguration->SourceToken,"VideoSource_1"); // must same to __tmd__GetVideoSources's token

    tProfile[0].VideoSourceConfiguration->UseCount = 1;

    tProfile[0].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap,sizeof(struct tt__IntRectangle));
    tProfile[0].VideoSourceConfiguration->Bounds->x = 0;
    tProfile[0].VideoSourceConfiguration->Bounds->y = 0;
    tProfile[0].VideoSourceConfiguration->Bounds->width = 1920;
    tProfile[0].VideoSourceConfiguration->Bounds->height = 1080;

    // audio source configuration

    tProfile[0].AudioSourceConfiguration = (struct tt__AudioSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__AudioSourceConfiguration ));
    soap_default_tt__AudioSourceConfiguration(soap,tProfile[0].AudioSourceConfiguration);

    tProfile[0].AudioSourceConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].AudioSourceConfiguration->Name,"AudioSourceConfig");

    tProfile[0].AudioSourceConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].AudioSourceConfiguration->token,"AudioSourceToken");

    tProfile[0].AudioSourceConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].AudioSourceConfiguration->SourceToken,"AudioSourceChannel1");

    tProfile[0].AudioSourceConfiguration->UseCount = 1;

    // video encoder configuration
    tProfile[0].VideoEncoderConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));
    soap_default_tt__VideoEncoderConfiguration(soap,tProfile[0].VideoEncoderConfiguration);

    tProfile[0].VideoEncoderConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].VideoEncoderConfiguration->Name,"VideoEncoder_1");

    tProfile[0].VideoEncoderConfiguration->token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].VideoEncoderConfiguration->token,"VideoEncoder_Token");

    tProfile[0].VideoEncoderConfiguration->UseCount = 1;
    tProfile[0].VideoEncoderConfiguration->Quality = 3.0;
    tProfile[0].VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264; //JPEG = 0, MPEG4 = 1, H264 = 2;

    tProfile[0].VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    tProfile[0].VideoEncoderConfiguration->Resolution->Width = 1920;
    tProfile[0].VideoEncoderConfiguration->Resolution->Height = 1080;

    tProfile[0].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    tProfile[0].VideoEncoderConfiguration->RateControl->FrameRateLimit = 30;
    tProfile[0].VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
    tProfile[0].VideoEncoderConfiguration->RateControl->BitrateLimit = 4096;

    // audio encoder configuration
    tProfile[0].AudioEncoderConfiguration = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__AudioEncoderConfiguration));
    soap_default_tt__AudioEncoderConfiguration(soap,tProfile[0].AudioEncoderConfiguration);

    tProfile[0].AudioEncoderConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].AudioEncoderConfiguration->Name,"AudioEncoder_1");

    tProfile[0].AudioEncoderConfiguration->token= (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tProfile[0].AudioEncoderConfiguration->token,"AudioEncoder_Token");

    tProfile[0].AudioEncoderConfiguration->UseCount = 1;
    tProfile[0].AudioEncoderConfiguration->Encoding = tt__AudioEncoding__G711;

    tProfile[0].AudioEncoderConfiguration->Bitrate = 64;
    tProfile[0].AudioEncoderConfiguration->SampleRate = 8;

    //tProfile[0].AudioEncoderConfiguration->SessionTimeout = 3000;

    trt__GetProfilesResponse->__sizeProfiles = ProfileSize;
    trt__GetProfilesResponse->Profiles = tProfile;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap* soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap* soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap* soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap* soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap* soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap* soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap* soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap* soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap* soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap* soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap* soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap* soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap* soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap* soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap* soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap* soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap* soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap* soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap* soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap* soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap* soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap* soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap* soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap* soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap* soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap* soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap* soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap* soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	struct tt__VideoSourceConfiguration *tConfiguration;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    // video source configuration
    tConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration ));
    soap_default_tt__VideoSourceConfiguration(soap,tConfiguration);

    tConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tConfiguration->Name,"VideoSourceConfig");

    tConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tConfiguration->token,"VideoSourceToken");

    tConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tConfiguration->SourceToken,"VideoSource_1"); // must same to __tmd__GetVideoSources's token

    tConfiguration->UseCount = 1;

    tConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap,sizeof(struct tt__IntRectangle));
    tConfiguration->Bounds->x = 0;
    tConfiguration->Bounds->y = 0;
    tConfiguration->Bounds->width = 1280;
    tConfiguration->Bounds->height = 720;

    trt__GetVideoSourceConfigurationResponse->Configuration = tConfiguration;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap* soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
	struct tt__VideoEncoderConfiguration *tConfiguration;
	tConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration ));
	soap_default_tt__VideoEncoderConfiguration(soap, tConfiguration);

	tConfiguration->Name = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
	strcpy(tConfiguration->Name,"VideoEncoderConfiguration");

	tConfiguration->token = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
	strcpy(tConfiguration->token,"VideoEncoderToken");

	tConfiguration->UseCount = 1;
	tConfiguration->Encoding = tt__VideoEncoding__H264;

	tConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
	tConfiguration->Resolution->Width = 1920;
	tConfiguration->Resolution->Height = 1080;

	tConfiguration->Quality = 3.0;

	tConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap,sizeof(struct tt__VideoRateControl));
	tConfiguration->RateControl->BitrateLimit = 4096;
	tConfiguration->RateControl->EncodingInterval = 1;
	tConfiguration->RateControl->FrameRateLimit = 30;

	trt__GetVideoEncoderConfigurationResponse->Configuration = tConfiguration;
	return SOAP_OK;

}

//SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap* soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap* soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
/*
{
	AUDIO_CFG_ST	stAudioConfig;
	MST_CFG_GET_AUDIO_CONFOG(&stAudioConfig);

	struct tt__AudioEncoderConfiguration *tConfiguration;
	tConfiguration = (struct tt__AudioEncoderConfiguration *)soap_malloc(soap,sizeof(struct tt__AudioEncoderConfiguration ));
	soap_default_tt__AudioEncoderConfiguration(soap, tConfiguration);
	tConfiguration->Encoding = stAudioConfig.Type;
	tConfiguration->Bitrate = stAudioConfig.Bitrate;
	tConfiguration->SampleRate = stAudioConfig.SampleBitrate;
	trt__GetAudioEncoderConfigurationResponse->Configuration = tConfiguration;
	return SOAP_OK;
}
*/
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap* soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap* soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap* soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap* soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap* soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap* soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap* soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap* soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap* soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
/*
{
	VIDEO_CFG_ST	stVideoCfg;
	int _with = 0;
	int _height = 0;

	int  nEncType  = 0;
	int  nEncSize  = 0;
	int	 nProfile  = 2;

	int nNewEncType = 0;
	int nNewWidth   = 0;
	int nNewHeight  = 0;
	if(trt__SetVideoEncoderConfiguration->Configuration != NULL)
	{
		GF_PARAM_GET_VIDEO_CONFIG(&stVideoCfg);
		if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == tt__VideoEncoding__JPEG)
			nEncType = 0;
		else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == tt__VideoEncoding__MPEG4)
			nEncType = 1;
		else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == tt__VideoEncoding__H264)
			nEncType = 0;
		if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
		{
			_with = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
			_height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
		}
		else
		{
			_with = stVideoCfg.stMainVEncCtrl.Width;
			_height = stVideoCfg.stMainVEncCtrl.Height;
		}
		if(_with == 1920)
			nEncSize = 0;
		else if(_with == 1280)
			nEncSize = 1;
		else
			nEncSize = 2;
		if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
		{
			stVideoCfg.stMainVEncCtrl.FrameRate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
			stVideoCfg.stMainVEncCtrl.KeyInterval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
			stVideoCfg.stMainVEncCtrl.Bitrate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
		}
		stVideoCfg.stMainVEncCtrl.Qulity = (UInt32)trt__SetVideoEncoderConfiguration->Configuration->Quality;
		stVideoCfg.stMainVEncCtrl.Qulity = stVideoCfg.stMainVEncCtrl.Qulity > 5 ? 5 : stVideoCfg.stMainVEncCtrl.Qulity;
		get_videosize_encoder(0, nEncType, nEncSize, &nNewEncType, &nNewWidth, &nNewHeight);
		stVideoCfg.stMainVEncCtrl.EncType    = nNewEncType;
		stVideoCfg.stMainVEncCtrl.Width      = nNewWidth;
		stVideoCfg.stMainVEncCtrl.Height     = nNewHeight;
		stVideoCfg.stMainVEncCtrl.Profile	 = nProfile;

		int copysync = 0;
		int nReboot_flag = 0;
		MST_CFG_SET_VIDEO_CONFIG(&stVideoCfg, &nReboot_flag);
		MST_VI_ChangeParam(&stVideoCfg, copysync, &nReboot_flag);
		if(nReboot_flag)
		{
			MST_FUNC_SetReboot(__FILE__,__LINE__);
		}
	}
	else
	{
		return SOAP_FAULT;
	}
	return SOAP_OK;
}
*/
//SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap* soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap* soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
/*
{
	AUDIO_CFG_ST	stAudioConfig;
	MST_CFG_GET_AUDIO_CONFOG(&stAudioConfig);

	if(trt__SetAudioEncoderConfiguration)
	{
		if(trt__SetAudioEncoderConfiguration->Configuration)
		{
			stAudioConfig.Type = trt__SetAudioEncoderConfiguration->Configuration->Encoding;
			stAudioConfig.Bitrate = trt__SetAudioEncoderConfiguration->Configuration->Bitrate;
			stAudioConfig.SampleBitrate = trt__SetAudioEncoderConfiguration->Configuration->SampleRate;
			MST_CFG_SET_AUDIO_CONFOG(&stAudioConfig);
		}
	}
	else
		return SOAP_FAULT;
	return SOAP_OK;
}
*/
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap* soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap* soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap* soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap* soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap* soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap* soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap* soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap* soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap* soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap* soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap* soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap* soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap* soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
	unsigned int V4Addr = 0;
    char IPAddr[COMMN_INFO_LEN];

    struct tt__MediaUri *tMediaUri;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetV4Ip(ETH_NAME, &V4Addr);
    sprintf(IPAddr, "rtsp://%s/main_stream", inet_ntoa(*((struct in_addr *)&V4Addr)));

    tMediaUri = (struct tt__MediaUri*)soap_malloc(soap, sizeof(struct tt__MediaUri));
    soap_default_tt__MediaUri(soap,tMediaUri);

    tMediaUri->Uri = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tMediaUri->Uri, IPAddr);

    tMediaUri->InvalidAfterConnect = xsd__boolean__false_;
    tMediaUri->InvalidAfterReboot = xsd__boolean__false_;

    trt__GetStreamUriResponse->MediaUri = tMediaUri;

    printf("[%d] %s exit !\n", __LINE__,__func__);
    return SOAP_OK;
}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap* soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap* soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap* soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap* soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	unsigned int V4Addr = 0;
    char IPAddr[COMMN_INFO_LEN];

    struct tt__MediaUri *tMediaUri;

    printf("[%d] %s enter !\n", __LINE__,__func__);

    mst_GetV4Ip(ETH_NAME, &V4Addr);
    sprintf(IPAddr, "http://%s:%d/onvif-http/snapshot", inet_ntoa(*((struct in_addr *)&V4Addr)), ONVIF_SERVICE_PORT);

    tMediaUri = (struct tt__MediaUri*)soap_malloc(soap, sizeof(struct tt__MediaUri));
    soap_default_tt__MediaUri(soap,tMediaUri);

    tMediaUri->Uri = (char *)soap_malloc(soap,sizeof(char)*SMALL_INFO_LEN);
    strcpy(tMediaUri->Uri, IPAddr);

    tMediaUri->InvalidAfterConnect = xsd__boolean__false_;
    tMediaUri->InvalidAfterReboot = xsd__boolean__false_;

    trt__GetSnapshotUriResponse->MediaUri = tMediaUri;

    printf("[%d] %s exit !\n", __LINE__,__func__);

    return SOAP_OK;
}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes(struct soap* soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode(struct soap* soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap* soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap* soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap* soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap* soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap* soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap* soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetServiceCapabilities(struct soap* soap, struct _trv__GetServiceCapabilities *trv__GetServiceCapabilities, struct _trv__GetServiceCapabilitiesResponse *trv__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceivers(struct soap* soap, struct _trv__GetReceivers *trv__GetReceivers, struct _trv__GetReceiversResponse *trv__GetReceiversResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceiver(struct soap* soap, struct _trv__GetReceiver *trv__GetReceiver, struct _trv__GetReceiverResponse *trv__GetReceiverResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__CreateReceiver(struct soap* soap, struct _trv__CreateReceiver *trv__CreateReceiver, struct _trv__CreateReceiverResponse *trv__CreateReceiverResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__DeleteReceiver(struct soap* soap, struct _trv__DeleteReceiver *trv__DeleteReceiver, struct _trv__DeleteReceiverResponse *trv__DeleteReceiverResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__ConfigureReceiver(struct soap* soap, struct _trv__ConfigureReceiver *trv__ConfigureReceiver, struct _trv__ConfigureReceiverResponse *trv__ConfigureReceiverResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__SetReceiverMode(struct soap* soap, struct _trv__SetReceiverMode *trv__SetReceiverMode, struct _trv__SetReceiverModeResponse *trv__SetReceiverModeResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __trv__GetReceiverState(struct soap* soap, struct _trv__GetReceiverState *trv__GetReceiverState, struct _trv__GetReceiverStateResponse *trv__GetReceiverStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetServiceCapabilities(struct soap* soap, struct _tse__GetServiceCapabilities *tse__GetServiceCapabilities, struct _tse__GetServiceCapabilitiesResponse *tse__GetServiceCapabilitiesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSummary(struct soap* soap, struct _tse__GetRecordingSummary *tse__GetRecordingSummary, struct _tse__GetRecordingSummaryResponse *tse__GetRecordingSummaryResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingInformation(struct soap* soap, struct _tse__GetRecordingInformation *tse__GetRecordingInformation, struct _tse__GetRecordingInformationResponse *tse__GetRecordingInformationResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMediaAttributes(struct soap* soap, struct _tse__GetMediaAttributes *tse__GetMediaAttributes, struct _tse__GetMediaAttributesResponse *tse__GetMediaAttributesResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindRecordings(struct soap* soap, struct _tse__FindRecordings *tse__FindRecordings, struct _tse__FindRecordingsResponse *tse__FindRecordingsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSearchResults(struct soap* soap, struct _tse__GetRecordingSearchResults *tse__GetRecordingSearchResults, struct _tse__GetRecordingSearchResultsResponse *tse__GetRecordingSearchResultsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindEvents(struct soap* soap, struct _tse__FindEvents *tse__FindEvents, struct _tse__FindEventsResponse *tse__FindEventsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetEventSearchResults(struct soap* soap, struct _tse__GetEventSearchResults *tse__GetEventSearchResults, struct _tse__GetEventSearchResultsResponse *tse__GetEventSearchResultsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindPTZPosition(struct soap* soap, struct _tse__FindPTZPosition *tse__FindPTZPosition, struct _tse__FindPTZPositionResponse *tse__FindPTZPositionResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetPTZPositionSearchResults(struct soap* soap, struct _tse__GetPTZPositionSearchResults *tse__GetPTZPositionSearchResults, struct _tse__GetPTZPositionSearchResultsResponse *tse__GetPTZPositionSearchResultsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetSearchState(struct soap* soap, struct _tse__GetSearchState *tse__GetSearchState, struct _tse__GetSearchStateResponse *tse__GetSearchStateResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__EndSearch(struct soap* soap, struct _tse__EndSearch *tse__EndSearch, struct _tse__EndSearchResponse *tse__EndSearchResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindMetadata(struct soap* soap, struct _tse__FindMetadata *tse__FindMetadata, struct _tse__FindMetadataResponse *tse__FindMetadataResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMetadataSearchResults(struct soap* soap, struct _tse__GetMetadataSearchResults *tse__GetMetadataSearchResults, struct _tse__GetMetadataSearchResultsResponse *tse__GetMetadataSearchResultsResponse){ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif


SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault_alex(struct soap* soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

////////////////////////////////////////////////////////////

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices_(struct soap* soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities_(struct soap* soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation_(struct soap* soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime_(struct soap* soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime_(struct soap* soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault_(struct soap* soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware_(struct soap* soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot_(struct soap* soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem_(struct soap* soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup_(struct soap* soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog_(struct soap* soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation_(struct soap* soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes_(struct soap* soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes_(struct soap* soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes_(struct soap* soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes_(struct soap* soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode_(struct soap* soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode_(struct soap* soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode_(struct soap* soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode_(struct soap* soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses_(struct soap* soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference_(struct soap* soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser_(struct soap* soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser_(struct soap* soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers_(struct soap* soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers_(struct soap* soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers_(struct soap* soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser_(struct soap* soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl_(struct soap* soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities_(struct soap* soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses_(struct soap* soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname_(struct soap* soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname_(struct soap* soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP_(struct soap* soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS_(struct soap* soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS_(struct soap* soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP_(struct soap* soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP_(struct soap* soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS_(struct soap* soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS_(struct soap* soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces_(struct soap* soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces_(struct soap* soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols_(struct soap* soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols_(struct soap* soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway_(struct soap* soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway_(struct soap* soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration_(struct soap* soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration_(struct soap* soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter_(struct soap* soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter_(struct soap* soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter_(struct soap* soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter_(struct soap* soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy_(struct soap* soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy_(struct soap* soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate_(struct soap* soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates_(struct soap* soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus_(struct soap* soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus_(struct soap* soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates_(struct soap* soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request_(struct soap* soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates_(struct soap* soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode_(struct soap* soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode_(struct soap* soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs_(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings_(struct soap* soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState_(struct soap* soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand_(struct soap* soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates_(struct soap* soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey_(struct soap* soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation_(struct soap* soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates_(struct soap* soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration_(struct soap* soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration_(struct soap* soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration_(struct soap* soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations_(struct soap* soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration_(struct soap* soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities_(struct soap* soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status_(struct soap* soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks_(struct soap* soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris_(struct soap* soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade_(struct soap* soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore_(struct soap* soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfigurations(struct soap* soap, struct _tds__GetStorageConfigurations *tds__GetStorageConfigurations, struct _tds__GetStorageConfigurationsResponse *tds__GetStorageConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateStorageConfiguration(struct soap* soap, struct _tds__CreateStorageConfiguration *tds__CreateStorageConfiguration, struct _tds__CreateStorageConfigurationResponse *tds__CreateStorageConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetStorageConfiguration(struct soap* soap, struct _tds__GetStorageConfiguration *tds__GetStorageConfiguration, struct _tds__GetStorageConfigurationResponse *tds__GetStorageConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetStorageConfiguration(struct soap* soap, struct _tds__SetStorageConfiguration *tds__SetStorageConfiguration, struct _tds__SetStorageConfigurationResponse *tds__SetStorageConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteStorageConfiguration(struct soap* soap, struct _tds__DeleteStorageConfiguration *tds__DeleteStorageConfiguration, struct _tds__DeleteStorageConfigurationResponse *tds__DeleteStorageConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities_(struct soap* soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources_(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources_(struct soap* soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs_(struct soap* soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile_(struct soap* soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile_(struct soap* soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles_(struct soap* soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration_(struct soap* soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration_(struct soap* soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration_(struct soap* soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration_(struct soap* soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration_(struct soap* soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration_(struct soap* soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration_(struct soap* soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration_(struct soap* soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration_(struct soap* soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration_(struct soap* soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration_(struct soap* soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration_(struct soap* soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration_(struct soap* soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration_(struct soap* soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration_(struct soap* soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration_(struct soap* soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration_(struct soap* soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration_(struct soap* soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile_(struct soap* soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations_(struct soap* soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations_(struct soap* soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations_(struct soap* soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations_(struct soap* soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations_(struct soap* soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations_(struct soap* soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations_(struct soap* soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations_(struct soap* soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration_(struct soap* soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration_(struct soap* soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration_(struct soap* soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration_(struct soap* soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration_(struct soap* soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration_(struct soap* soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration_(struct soap* soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration_(struct soap* soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations_(struct soap* soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations_(struct soap* soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations_(struct soap* soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations_(struct soap* soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations_(struct soap* soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations_(struct soap* soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations_(struct soap* soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations_(struct soap* soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration_(struct soap* soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration_(struct soap* soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
//SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration_(struct soap* soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
	//{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration_(struct soap* soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration_(struct soap* soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration_(struct soap* soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration_(struct soap* soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration_(struct soap* soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions_(struct soap* soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions_(struct soap* soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions_(struct soap* soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions_(struct soap* soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions_(struct soap* soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions_(struct soap* soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions_(struct soap* soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances_(struct soap* soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri_(struct soap* soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming_(struct soap* soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming_(struct soap* soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint_(struct soap* soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri_(struct soap* soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#if 0 //struct not declared
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes_(struct soap* soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode_(struct soap* soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs_(struct soap* soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD_(struct soap* soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions_(struct soap* soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD_(struct soap* soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD_(struct soap* soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD_(struct soap* soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse)
	{ONVIF_NOTSUPPORTED_FUNC(soap, NULL);}
#endif































