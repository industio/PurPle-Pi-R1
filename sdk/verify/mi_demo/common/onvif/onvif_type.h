/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (??Sigmastar Confidential Information??) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _ONVIF_TYPE_H
#define _ONVIF_TYPE_H
/*
#include <time.h>
#include "global.h"

#define	ONVIF_MAX_ADDR_LEN     128		//63 chiness word£¬127 Character
#define	ONVIF_MAX_TOKEN_LEN	   64		//31 chiness word£¬63 Character
#define	ONVIF_MAX_STR_LEN	   64		//31 chiness word£¬63 Character
#define ONVIF_MAX_SCOPES	   8
#define MAX_SUBSCRIBE_USER     10
#define USER_NAME_LEN 	       16
#define USER_PASSWD_LEN   	   16
#define	DOMAIN_NAME_LEN		   32

typedef struct _ONVIF_GetDeviceTpye_S
{
	int	devicetype;
	int	VideoNum;
	int ipcoripc;

}ONVIF_GetDeviceTpye_S;	//ContinuousFocus

typedef struct _ONVIF_GetOutputStatus_S
{
	unsigned char nOutputStatus;
	unsigned int AutoClearTime;	
	unsigned char nOutputMode;
	
}ONVIF_GetOutputStatus_S;	

//--------ImagingSettings-----------------

typedef struct _ONVIF_FloatRange_S
{
	float  Min;
	float  Max;
}ONVIF_FloatRange_S;	//FloatRange

typedef	struct	_ONVIF_WhiteBalance_S
{
	char		Mode[ONVIF_MAX_TOKEN_LEN];		//AUTO|MANUAL
	int			CrGain;
	int			CbGain;
}ONVIF_WhiteBalance_S;

typedef struct	_ONVIF_WideDynamicRange_S
{
	char		Mode[ONVIF_MAX_TOKEN_LEN];		//OFF|ON
	int			Level;
}ONVIF_WideDynamicRange_S;

typedef struct	_ONVIF_FocusCfg_S
{
	char		AutoFocusMode[ONVIF_MAX_TOKEN_LEN];	//AUTO|MANUAL
	int			DefaultSpeed;
	int 		NearLimit;
    int         FarLimit;
}ONVIF_FocusCfg_S;	//FocusConfiguration

typedef struct	_ONVIF_Rectangle_S
{
	int		bottom;
	int		top;
	int		right;
	int		left;

}ONVIF_Rectangle_S;	//Rectangle

typedef struct	_ONVIF_Exposure_S
{
	char				Mode[ONVIF_MAX_TOKEN_LEN];		//AUTO|MANUAL
	char				Priority[ONVIF_MAX_TOKEN_LEN];	//LowNoise|FrameRate
	ONVIF_Rectangle_S	Window;
	int					MinExposureTime;
	int 				MaxExposureTime;
	int					MinGain;
	int					MaxGain;
	int					MinIris;
	int					MaxIris;
	int					ExposureTime;
	int					Gain;
	int					Iris;
	
}ONVIF_Exposure_S;	//Exposure

typedef struct	_ONVIF_BacklightCompensation_S
{
	char		Mode[ONVIF_MAX_TOKEN_LEN];		//OFF|ON
	int			Level;
}ONVIF_BacklightCompensation_S;	//BacklightCompensation

typedef struct _ONVIF_ImagingSettings_S
{
	char							VideoSourceToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_BacklightCompensation_S	BacklightCompensation;
	int								Brightness;
	int								ColorSaturation;
	int								Contrast;
	ONVIF_Exposure_S				Exposure;
	ONVIF_FocusCfg_S				Focus;
	char							IrCutFilter[ONVIF_MAX_TOKEN_LEN];	//ON"|OFF|AUTO
	int								Sharpness;
	ONVIF_WideDynamicRange_S		WideDynamicRange;
	ONVIF_WhiteBalance_S			WhiteBalance;
	char                                                         chn;
	char                                                         devicetype;

}ONVIF_ImagingSettings_S;	

//-------------ImagingOptions----------------------
typedef struct _ONVIF_WhiteBalanceOptions_S
{
	char					Mode[ONVIF_MAX_TOKEN_LEN];	//AUTO|MANUAL
	ONVIF_FloatRange_S		YrGain;
	ONVIF_FloatRange_S		YbGain;
}ONVIF_WhiteBalanceOptions_S;	//WhiteBalanceOptions

typedef struct _ONVIF_WideDynamicRangeOptions_S
{
	char					Mode[ONVIF_MAX_TOKEN_LEN];	//OFF|ON
	ONVIF_FloatRange_S		Level;
}ONVIF_WideDynamicRangeOptions_S;	//WideDynamicRangeOptions


typedef struct _ONVIF_FocusOptions_S
{
	char					AutoFocusModes[ONVIF_MAX_TOKEN_LEN];	//AUTO|MANUAL
	ONVIF_FloatRange_S		DefaultSpeed;
	ONVIF_FloatRange_S		NearLimit;
	ONVIF_FloatRange_S		FarLimit;
}ONVIF_FocusOptions_S;	//FocusOptions

typedef struct _ONVIF_ExposureOptions_S
{
	char					Mode[ONVIF_MAX_TOKEN_LEN];	//AUTO|MANUAL
	char					Priority[ONVIF_MAX_TOKEN_LEN];	//LowNoise|FrameRate
	ONVIF_FloatRange_S		MinExposureTime;
	ONVIF_FloatRange_S		MaxExposureTime;
	ONVIF_FloatRange_S		MinGain;
	ONVIF_FloatRange_S		MaxGain;
	ONVIF_FloatRange_S		MinIris;
	ONVIF_FloatRange_S		MaxIris;
	ONVIF_FloatRange_S		ExposureTime;
	ONVIF_FloatRange_S		Gain;
	ONVIF_FloatRange_S		Iris;
}ONVIF_ExposureOptions_S;	//ExposureOptions

typedef struct _ONVIF_BacklightCompensationOptions_S
{
	char					Mode[ONVIF_MAX_TOKEN_LEN];	//OFF|ON
	ONVIF_FloatRange_S		Level;

}ONVIF_BacklightCompensationOptions_S;	//BacklightCompensationOptions

typedef struct _ONVIF_ImagingOptions_S
{
	char									VideoSourceToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_BacklightCompensationOptions_S	BacklightCompensation;
	ONVIF_FloatRange_S						Brightness;
	ONVIF_FloatRange_S						ColorSaturation;
	ONVIF_FloatRange_S						Contrast;
	ONVIF_ExposureOptions_S					Exposure;
	ONVIF_FocusOptions_S					Focus;
	char									IrCutFilterModes[ONVIF_MAX_TOKEN_LEN];	//ON|OFF|AUTO
	ONVIF_FloatRange_S						Sharpness;
	ONVIF_WideDynamicRangeOptions_S			WideDynamicRange;
	ONVIF_WhiteBalanceOptions_S				WhiteBalance;

}ONVIF_ImagingOptions_S;	//ImagingOptions

//---------------ImagingStatus---------------------
typedef struct ONVIF__S
{
	int			Position;
	char		MoveStatus[ONVIF_MAX_TOKEN_LEN];	//IDLE|MOVING|UNKNOWN
	char		Error[ONVIF_MAX_TOKEN_LEN];
}ONVIF_FocusStatus_S;	//FocusStatus

typedef struct _ONVIF_ImagingStatus_S
{
	char				VideoSourceToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_FocusStatus_S	FocusStatus;
}ONVIF_ImagingStatus_S;	//ImagingStatus

typedef enum  _GF_ONVIF_AUDIOENCODER_E
{
	GF_ONVIF_AENC_NONE   = 0x0,
	GF_ONVIF_AENC_G726   = 0x01,
	GF_ONVIF_AENC_G722   = 0x02,
	GF_ONVIF_AENC_G711A  = 0x03,
	GF_ONVIF_AENC_ADPCM  = 0x04,
	GF_ONVIF_AENC_MP3    = 0x05,
	GF_ONVIF_AENC_PCM    = 0x06,
	GF_ONVIF_AENC_G711U  = 0x07,
	GF_ONVIF_AENC_AACLC  = 0x08,
	GF_ONVIF_AENC_AMRNB  = 0x09,

}GF_ONVIF_AUDIOENCODER_E;

typedef struct _ONVIF_AudioParm_S
{
	
	GF_ONVIF_AUDIOENCODER_E		Encoding; 	//G711|G726|AAC
	int	BitrateList;
	int	SampleRateList;
	int	chn;
}ONVIF_AudioParm_S;

typedef struct GF_PRESET_INFO
{
	char   szTitle[32];	
	unsigned char  nUserPresetNo;		
} GF_PRESET_INFO;

typedef struct GF_GET_PRESET
{
	unsigned short presetnum;	
	//GF_PRESET_INFO  stpresetinfo[110];
	GF_PRESET_INFO  stpresetinfo[255];
} GF_GET_PRESET;

//video config
typedef struct _ONVIF_VIDEO_CONF_S
{
	unsigned char h264profile;
	unsigned char chn;
	unsigned char catype;
	unsigned char entype;
	unsigned char piclevel;
	unsigned char cbr;
	int width;
	int height;
	int bitrate;
	int fps;
	int gop;
	int rebootflag;
	
}ONVIF_VIDEO_CONF_S;

typedef struct _ONVIF_RTSP_MULT_S
{
	unsigned char 	  multi_ttl;
	unsigned char     mode;
	char           	  recv[3];
	unsigned int      multip;
	unsigned short    privport;      // primary video port
	unsigned short    priaport;      // primary audio port
	unsigned short    secvport;      // secondary video port
	unsigned short    secaport;      // secondary audio port

}ONVIF_RTSP_MULT_S;

typedef struct _ONVIF_PTZ_CAPABILITIES_S
{
	char			XAddr[ONVIF_MAX_ADDR_LEN];
}ONVIF_PTZ_CAPABILITIES_S;				//PTZCapabilities;

typedef struct _ONVIF_REALTIMESTREAM_CAPABILITIES_S
{
	unsigned char	RTPMulticast;
	unsigned char	RTP_TCP;
	unsigned char	RTP_RTSP_TCP;
	unsigned char   reserved;
}ONVIF_REALTIMESTREAM_CAPABILITIES_S;	//RealTimeStreamingCapabilities;

typedef struct _ONVIF_MEDIA_CAPABILITY_S
{
	char									XAddr[ONVIF_MAX_ADDR_LEN];
	ONVIF_REALTIMESTREAM_CAPABILITIES_S		StreamingCapabilities;
	
}ONVIF_MEDIA_CAPABILITY_S;				//MediaCapabilities;

typedef struct _ONVIF_IMAGE_CAPABILITY_S
{
	char			XAddr[ONVIF_MAX_ADDR_LEN];
	
}ONVIF_IMAGE_CAPABILITY_S;				//ImagingCapabilities;

typedef struct _ONVIF_EVENT_CAPABILITY_S
{
	char			XAddr[ONVIF_MAX_ADDR_LEN];
	unsigned char	WSSubscriptionPolicySupport;
	unsigned char	WSPullPointSupport;
	unsigned char	WSPausableSubscriptionManagerInterfaceSupport;
	
}ONVIF_EVENT_CAPABILITY_S;				//EventsCapabilities;

typedef struct _ONVIF_SECURITY_CAPABILITY_S
{
	unsigned char			TLS11;
	unsigned char			TLS12;
	unsigned char			OnboardKeyGeneration;
	unsigned char			AccessPolicyConfig;
	unsigned char			X509Token;
	unsigned char			SAMLToken;
	unsigned char			KerberosToken;
	unsigned char			RELToken;
}ONVIF_SECURITY_CAPABILITY_S;			//SecurityCapabilities;

typedef struct _ONVIF_IO_CAPABILITY_S
{
	unsigned int			InputConnectors;
	unsigned int			RelayOutputs;
	
}ONVIF_IO_CAPABILITY_S;					//IOCapabilities;


typedef struct _ONVIF_VERSION_S			
{
	unsigned int	Major;
	unsigned int	Minor;
}ONVIF_VERSION_S;						//OnvifVersion;

typedef struct _ONVIF_SYSTEM_CAPABILITY_S
{
	unsigned char			DiscoveryResolve;
	unsigned char			DiscoveryBye;
	unsigned char			RemoteDiscovery;
	unsigned char			SystemBackup;
	unsigned char			SystemLogging;
	unsigned char			FirmwareUpgrade;
	ONVIF_VERSION_S			SupportedVersions;
}ONVIF_SYSTEM_CAPABILITY_S;				//SystemCapabilities;

typedef struct _ONVIF_NETWORK_CAPABILITY_S
{
	unsigned char			IPFilter;
	unsigned char			ZeroConfiguration;
	unsigned char			IPVersion6;
	unsigned char			DynDNS;
	
}ONVIF_NETWORK_CAPABILITY_S;			//NetworkCapabilities;

typedef struct _ONVIF_DEVICE_CAPABILITY_S
{
	char							XAddr[ONVIF_MAX_ADDR_LEN];
	ONVIF_NETWORK_CAPABILITY_S		Network;
	ONVIF_SYSTEM_CAPABILITY_S		System;
	ONVIF_IO_CAPABILITY_S			IO;
	ONVIF_SECURITY_CAPABILITY_S		Security;
}ONVIF_DEVICE_CAPABILITY_S;				//DeviceCapabilities;

typedef struct _ONVIF_ANALYTICS_CAPABILITY_S
{
	char							XAddr[ONVIF_MAX_ADDR_LEN];
	unsigned char					RuleSupport;
	unsigned char					AnalyticsModuleSupport;

}ONVIF_ANALYTICS_CAPABILITY_S;			//AnalyticsCapabilities;

typedef struct _ONVIF_CAPABILITY_S
{
	ONVIF_ANALYTICS_CAPABILITY_S	Analytics;	
	ONVIF_DEVICE_CAPABILITY_S		Device;				
	ONVIF_EVENT_CAPABILITY_S		Events;				
	ONVIF_IMAGE_CAPABILITY_S		Imaging;			
	ONVIF_MEDIA_CAPABILITY_S		Media;					
	ONVIF_PTZ_CAPABILITIES_S		PTZ;				
	
	unsigned int					Extension;				
}ONVIF_CAPABILITY_S;					//Capabilities;

typedef struct _ONVIF_HOSTNAME_INFO_S
{
	unsigned int			FromDHCP;
	char					Name[ONVIF_MAX_TOKEN_LEN+1];
	
	unsigned int			Extension;	//À©Õ¹
}ONVIF_HOSTNAME_INFO_S;					//HostNameInfo;

typedef struct _ONVIF_IPADDRESS_S
{
	char			Type[ONVIF_MAX_TOKEN_LEN];			//IPv6|IPv4
	char			IPv4Address[ONVIF_MAX_TOKEN_LEN];
	char			IPv6Address[ONVIF_MAX_TOKEN_LEN];
}ONVIF_IPADDRESS_S;						//IPAddress;

typedef struct _ONVIF_DNS_INFO_S
{
	unsigned int			FromDHCP;
	char					SearchDomain[ONVIF_MAX_TOKEN_LEN];
	ONVIF_IPADDRESS_S		DNSFromDHCP;
	ONVIF_IPADDRESS_S		DNSManual;

	unsigned int			Extension;	//À©Õ¹
}ONVIF_DNS_INFO_S;						//DNSInfo;

typedef struct _ONVIF_NETWORK_HOST_S
{
	char			Type[ONVIF_MAX_TOKEN_LEN];	//IPv4|IPv6|DNS
	char			IPv4Address[ONVIF_MAX_TOKEN_LEN];
	char			IPv6Address[ONVIF_MAX_TOKEN_LEN];
	char			DNSname[ONVIF_MAX_TOKEN_LEN];
}ONVIF_NETWORK_HOST_S;					//NetworkHost

typedef struct _ONVIF_NTP_CFG_ST_S
{
	unsigned int			ntpOpen;					
	unsigned int			ntpTimeZone;				
	char					ntpHost[DOMAIN_NAME_LEN];	
	unsigned int			nDaylight;
}ONVIF_NTP_CFG_ST_S;

typedef struct _ONVIF_DEVICE_INFO_S
{
	char				Manufacturer[ONVIF_MAX_STR_LEN];
	char				Model[ONVIF_MAX_STR_LEN];
	char				FirmwareVersion[ONVIF_MAX_STR_LEN];
	char				SerialNumber[ONVIF_MAX_STR_LEN];
	char				HardwareId[ONVIF_MAX_STR_LEN];
	
}ONVIF_DEVICE_INFO_S;					//DeviceInfo;

typedef struct _ONVIF_DATE_S
{
	unsigned int			Year;
	unsigned char			Month;
	unsigned char			Day;
	unsigned char			rev1;
	unsigned char			rev2;
}ONVIF_DATE_S;							//Date;

typedef struct
{
	int hour;
	int minute;
	int second;
	int year;
	int month;
	int day;
	
}ONVIF_SYSDATE_TIME;

typedef struct _ONVIF_SCOPE_S
{
	int		nScopeDef;	//[ONVIF_MAX_TOKEN_LEN];	//"Fixed|Configurable"  0: Fixed 1: Configurable
	char	ScopeItem[ONVIF_MAX_TOKEN_LEN];
	
}ONVIF_SCOPE_S;							//Scope;

typedef struct _ONVIF_SCOPES_S
{
	ONVIF_SCOPE_S	Scopes[ONVIF_MAX_SCOPES];
	int 			ScopesNum;
	
}ONVIF_SCOPES_S;						//Scopes;

typedef struct _ONVIF_DDNS_INFO_S
{
	char	Type[ONVIF_MAX_TOKEN_LEN];		//NoUpdate|ClientUpdates|ServerUpdates
	char	Name[ONVIF_MAX_TOKEN_LEN];
	char	TTL[ONVIF_MAX_TOKEN_LEN];		//duration
	
}ONVIF_DDNS_INFO_S;						//DyDNSInfo;

typedef struct _ONVIF_PREFIXED_IPV6_S
{
	char		Address[ONVIF_MAX_TOKEN_LEN];
	int			PrefixLength;
}ONVIF_PREFIXED_IPV6_S;					//PrefixedIPv6Address;

typedef struct _ONVIF_IPV6_CONFIG_S
{
	int						AcceptRouterAdvert;
	char					DHCP[ONVIF_MAX_TOKEN_LEN];	//IPv6DHCPConfiguration Auto|Stateful|Stateless|Off
	ONVIF_PREFIXED_IPV6_S	Manual;
	ONVIF_PREFIXED_IPV6_S	LinkLocal;
	ONVIF_PREFIXED_IPV6_S	FromDHCP;
	ONVIF_PREFIXED_IPV6_S	FromRA;
}ONVIF_IPV6_CONFIG_S;					//IPv6Configuration;

typedef struct _ONVIF_IPV6_IF_S
{
	int						Enabled;
	ONVIF_IPV6_CONFIG_S		Config;
}ONVIF_IPV6_IF_S;						//IPv6NetworkInterface;

typedef struct _ONVIF_PREFIXED_IPV4_S
{
	char		Address[ONVIF_MAX_TOKEN_LEN];
	char		PrefixLength;
}ONVIF_PREFIXED_IPV4_S;					//PrefixedIPv4Address;

typedef struct _ONVIF_IPV4_CONFIG_S
{
	ONVIF_PREFIXED_IPV4_S		Manual;
	ONVIF_PREFIXED_IPV4_S		LinkLocal;
	ONVIF_PREFIXED_IPV4_S		FromDHCP;
	int							DHCP;
}ONVIF_IPV4_CONFIG_S;					//IPv4Configuration;

typedef struct _ONVIF_IPV4_IF_S
{
	int							Enabled;
	ONVIF_IPV4_CONFIG_S			Config;
}ONVIF_IPV4_IF_S;				//IPv4NetworkInterface;

typedef struct _ONVIF_NETIF_CONNECTION_CONFIG_S
{
	int		AutoNegotiation;
	int		Speed;
	char	Duplex[ONVIF_MAX_TOKEN_LEN];	//Full|Half
	
}ONVIF_NETIF_CONNECTION_CONFIG_S;//NetworkInterfaceConnectionSetting;

typedef struct _ONVIF_NETINTERFACE_LINK_S
{
	ONVIF_NETIF_CONNECTION_CONFIG_S	AdminSettings;
	ONVIF_NETIF_CONNECTION_CONFIG_S	OperSettings;
	int	InterfaceType;   //For valid numbers, please refer to http://www.iana.org/assignments/ianaiftype-mib.
	
}ONVIF_NETIF_LINK_S;				//NetworkInterfaceLink;

typedef struct _ONVIF_NETIF_INFO_S
{
	char	Name[ONVIF_MAX_TOKEN_LEN];
	char	HwAddress[ONVIF_MAX_TOKEN_LEN];
	int		MTU;
	
}ONVIF_NETIF_INFO_S;				//NetworkInterfaceInfo;

typedef struct _ONVIF_NETIF_S
{
	char	DeviceEntity[ONVIF_MAX_TOKEN_LEN];
	char	Enabled;
	ONVIF_NETIF_INFO_S			Info;
	ONVIF_NETIF_LINK_S			Link;
	ONVIF_IPV4_IF_S				IPv4;
	ONVIF_IPV6_IF_S				IPv6;
}ONVIF_NETIF_S;						//NetworkInterface;

typedef struct _ONVIF_NET_PROTOCOL_S
{
	char	Name[ONVIF_MAX_TOKEN_LEN];	//HTTP|HTTPS|RTSP
	int		Enabled;
	int		Port;
}ONVIF_NET_PROTOCOL_S;				//NetworkProtocol;

typedef struct _ONVIF_NET_GATEWAY_S
{
	char	IPv4Address[ONVIF_MAX_TOKEN_LEN];
	char	IPv6Address[ONVIF_MAX_TOKEN_LEN];
}ONVIF_NET_GATEWAY_S;				//NetworkGateway;

typedef struct _ONVIF_ZERO_CONFIG_S
{
	char	InterfaceToken[ONVIF_MAX_TOKEN_LEN];
	char	Enabled;
	char	Addresses[ONVIF_MAX_TOKEN_LEN];
}ONVIF_ZERO_CONFIG_S;				//ZeroConfiguration;

typedef struct _ONVIF_IPADDRESS_FILTER_S
{
	char					Type[ONVIF_MAX_TOKEN_LEN];	//Allow|Deny
	ONVIF_PREFIXED_IPV4_S	IPv4Address;
	ONVIF_PREFIXED_IPV6_S	IPv6Address;
}ONVIF_IPADDRESS_FILTER_S;			//IPAddressFilter;

typedef struct _ONVIF_BACKUPFILES_S
{
	char	Name[ONVIF_MAX_TOKEN_LEN];
	char	Date[ONVIF_MAX_TOKEN_LEN];
}ONVIF_BACKUPFILES_S;				//BackupFiles;

// Media type
typedef struct _ONVIF_ADDRESS_S
{
	char		Type[ONVIF_MAX_STR_LEN];
	char		IPv4Address[ONVIF_MAX_TOKEN_LEN];
	char		IPv6Address[ONVIF_MAX_TOKEN_LEN];
}ONVIF_ADDRESS_S;					//Address;

typedef struct _ONVIF_MULTICAST_S
{
	ONVIF_ADDRESS_S		Address;
	char				TTL[ONVIF_MAX_STR_LEN];
	int					AutoStart;
	int					Port;
}ONVIF_MULTICAST_S;					//Multicast;

typedef struct _ONVIF_PTZSTATUS_S
{
	int				Status;
	int				Position;
}ONVIF_PTZSTATUS_S;					//PTZStatus;

typedef struct _ONVIF_METADATA_CONFIG_S
{
	char					token[ONVIF_MAX_STR_LEN];
	char					Name[ONVIF_MAX_STR_LEN];
	int						UseCount;
	ONVIF_PTZSTATUS_S		PTZStatus;
	char					Events[ONVIF_MAX_STR_LEN];
	int						Analytics;
	ONVIF_MULTICAST_S		Multicast;
	char					SessionTimeout[ONVIF_MAX_STR_LEN];
}ONVIF_METADATA_CONFIG_S;			//MetadataConfiguration;

typedef struct _ONVIF_RANGE_S
{
	float		Min;
	float		Max;
}ONVIF_RANGE_S;						//Range;

typedef struct _ONVIF_ZLRANGE_S
{
	char				URI[ONVIF_MAX_TOKEN_LEN];
	ONVIF_RANGE_S		XRange;
}ONVIF_ZLRANGE_S;					//ZLRange;

typedef struct _ONVIF_ZOOMLIMITS_S
{
	ONVIF_ZLRANGE_S		Range;
}ONVIF_ZOOMLIMITS_S;				//ZoomLimits;

typedef struct _ONVIF_PTLRANGE_S
{
	char				URI[ONVIF_MAX_TOKEN_LEN];
	ONVIF_RANGE_S		XRange;
	ONVIF_RANGE_S		YRange;
}ONVIF_PTLRANGE_S;					//PTLRange;

typedef struct _ONVIF_PANTILTLIMITS_S
{
	ONVIF_PTLRANGE_S	Range;
}ONVIF_PANTILTLIMITS_S;				//PanTiltLimits;

typedef struct _ONVIF_ZOOM_S
{
	int	x;
}ONVIF_ZOOM_S;						//Zoom;

typedef struct _ONVIF_PANTILT_S
{
	int		x;
	int		y;
}ONVIF_PANTILT_S;					//PanTilt;

typedef struct _ONVIF_DEFAULTPTZSPEED_S
{
	ONVIF_PANTILT_S			PanTilt;
	ONVIF_ZOOM_S 			Zoom;
}ONVIF_DEFAULTPTZSPEED_S;			//DefaultPTZSpeed;

typedef struct _ONVIF_PTZ_CONFIG_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	int							UseCount;
	char						NodeToken[ONVIF_MAX_STR_LEN];
	char						DefaultAbsolutePantTiltPositionSpace[ONVIF_MAX_STR_LEN];
	char						DefaultAbsoluteZoomPositionSpace[ONVIF_MAX_STR_LEN];
	char						DefaultRelativePanTiltTranslationSpace[ONVIF_MAX_STR_LEN];
	char						DefaultRelativeZoomTranslationSpace[ONVIF_MAX_STR_LEN];
	char						DefaultContinuousPanTiltVelocitySpace[ONVIF_MAX_STR_LEN];
	char						DefaultContinuousZoomVelocitySpace[ONVIF_MAX_STR_LEN];
	ONVIF_DEFAULTPTZSPEED_S		DefaultPTZSpeed;
	char						DefaultPTZTimeout[ONVIF_MAX_STR_LEN];
	ONVIF_PANTILTLIMITS_S		PanTiltLimits;
	ONVIF_ZOOMLIMITS_S			ZoomLimits;
}ONVIF_PTZ_CONFIG_S;				//PTZConfiguration;

typedef struct _ONVIF_SIMPLEITEM_S
{
	char				Name[ONVIF_MAX_STR_LEN];
	char				Value[ONVIF_MAX_STR_LEN];
}ONVIF_SIMPLEITEM_S;				//SimpleItem;

typedef struct _ONVIF_ENGINE_CONFIG_S
{
	char				Name[ONVIF_MAX_STR_LEN];
	char				Type[ONVIF_MAX_STR_LEN];
	ONVIF_SIMPLEITEM_S	SimpleItem;

}ONVIF_ENGINE_CONFIG_S;				//EngineConfig;

typedef struct _ONVIF_RULE_ENGINE_CONFIG_S
{
	ONVIF_ENGINE_CONFIG_S		Rule;

}ONVIF_RULE_ENGINE_CONFIG_S;		//RuleEngineConfiguration;

typedef struct _ONVIF_ANALYTICS_ENGINE_CONFIG_S
{
	ONVIF_ENGINE_CONFIG_S		AnalyticsModule;

}ONVIF_ANALYTICS_ENGINE_CONFIG_S;	//AnalyticsEngineConfiguration;

typedef struct _ONVIF_ViANALYTICS_CONFIG_S
{
	char							token[ONVIF_MAX_STR_LEN];
	char							Name[ONVIF_MAX_STR_LEN];
	int								UseCount;
	ONVIF_ANALYTICS_ENGINE_CONFIG_S	AnalyticsEngineConfiguration;
	ONVIF_RULE_ENGINE_CONFIG_S		RuleEngineConfiguration;

}ONVIF_ViANALYTICS_CONFIG_S;		//VideoAnalyticsConfiguration;

typedef struct _ONVIF_AENC_CONFIG_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	int						    UseCount;
	char						Encoding[ONVIF_MAX_STR_LEN];
	int						    Bitrate;
	int 						SampleRate;
	ONVIF_MULTICAST_S			Multicast;
	char						SessionTimeout[ONVIF_MAX_STR_LEN];
	int							ForcePersistence;
}ONVIF_AENC_CONFIG_S;			//AudioEncoderConfiguration;

typedef struct _ONVIF_H264_S
{
	int			GovLength;
	char		H264Profile[ONVIF_MAX_STR_LEN];
}ONVIF_H264_S;						//H264;

typedef struct _ONVIF_MPEG4_S
{
	int			GovLength;
	char		Mpeg4Profile[ONVIF_MAX_STR_LEN];
}ONVIF_MPEG4_S;						//MPEG4;

typedef struct _ONVIF_RATECONTROL_S
{
	int			FrameRateLimit;
	int			EncodingInterval;
	int			BitrateLimit;
}ONVIF_RATECONTROL_S;				//RateControl;

typedef struct _ONVIF_INT_RANGE_S
{
	int lower;
	int higher;
}ONVIF_INT_RANGE_S;

typedef struct _ONVIF_RESOLUTION_S
{
	int			Width;
	int			Height;
}ONVIF_RESOLUTION_S;				//Resolution;

typedef struct _ONVIF_VENC_CONFIG_OPTIONS_S
{
	int                 index;
	ONVIF_RESOLUTION_S  resol[4];
	ONVIF_INT_RANGE_S   quility;
	ONVIF_INT_RANGE_S   frameRate;
	ONVIF_INT_RANGE_S   encodingInterval;
	ONVIF_INT_RANGE_S   bitRate;
	ONVIF_INT_RANGE_S   h264GOVLen;
	char                h264Profile[12]; // 'Baseline', 'Main', 'Extended', 'High'
	char                profileToken[ONVIF_MAX_STR_LEN];
}ONVIF_VENC_CFG_OPTS_S;

typedef struct _ONVIF_VENC_CONFIG_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	int							UseCount;
	char						Encoding[ONVIF_MAX_STR_LEN];
	ONVIF_RESOLUTION_S			Resolution;
	int							Quality;
	ONVIF_RATECONTROL_S			RateControl;
	ONVIF_MPEG4_S				MPEG4;
	ONVIF_H264_S				H264;
	ONVIF_MULTICAST_S			Multicast;
	char						SessionTimeout[ONVIF_MAX_STR_LEN];

}ONVIF_VENC_CONFIG_S;		//VideoEncoderConfiguration;

typedef struct _ONVIF_AI_CONFIG_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	int							UseCount;
	char						SourceToken[ONVIF_MAX_STR_LEN];
	int							ForcePersistence;
}ONVIF_AI_CONFIG_S;				//AudioSourceConfiguration;

typedef struct _ONVIF_BOUNDS_S
{
	int		x;
	int		y;
	int		width;
	int		height;
}ONVIF_BOUNDS_S;				//Bounds;

typedef struct _ONVIF_VI_CONFIG_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	int							UseCount;
	char						SourceToken[ONVIF_MAX_STR_LEN];
	ONVIF_BOUNDS_S				Bounds;
}ONVIF_VI_CONFIG_S;				//VideoSourceConfiguration;

typedef struct _ONVIF_Profile_S
{
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
}ONVIF_Profile_S;

typedef struct _ONVIF_PROFILES_S
{
	char						index;
	char						res[3];
	char						token[ONVIF_MAX_STR_LEN];
	char						Name[ONVIF_MAX_STR_LEN];
	ONVIF_VI_CONFIG_S			ViConfig;
	ONVIF_AI_CONFIG_S			AiConfig;
	ONVIF_VENC_CONFIG_S			VencConfig;
	ONVIF_AENC_CONFIG_S			AencConfig;
	ONVIF_PTZ_CONFIG_S			PTZConfig;
}ONVIF_PROFILES_S;				//Profiles;
	
typedef struct _ONVIF_MEDIAURI_S
{
	char			Uri[ONVIF_MAX_STR_LEN];
	char			InvalidAfterConnect;
	char			InvalidAfterReboot;
    char            recv[2];
	char			Timeout[ONVIF_MAX_STR_LEN];
}ONVIF_MEDIAURI_S;				//MediaUri;

typedef struct ONVIF_NETWORK_DHCP_S
{
	unsigned char networkdhcp;
	
}ONVIF_NETWORK_DHCP_S;				//NetworkGateway

typedef struct ONVIF_NETWORK_PROTOCOL_S
{
	unsigned short  httpport;
	unsigned short  rtspport;
}ONVIF_NETWORK_PROTOCOL_S;			//NetworkGateway

typedef struct ONVIF__USER_CONFIG_S
{
	unsigned char	usernum;
	unsigned int  	Level;							//priority
	char 			Name[USER_NAME_LEN + 1];		//user name
	char 			Password[USER_NAME_LEN + 1];	//password
	unsigned char	iemethod;
	unsigned char	AD[1];
	
}ONVIF_USER_CONFIG_S ;

typedef struct _ONVIF_Vector1D_S
{
    float   x;
    char    space[ONVIF_MAX_ADDR_LEN];
}ONVIF_Vector1D_S;		//Vector1D

typedef struct _ONVIF_Vector2D_S
{
    float   x;
    float   y;
	char	space[ONVIF_MAX_ADDR_LEN];
}ONVIF_Vector2D_S;		//Vector2D

typedef struct _ONVIF_PTZSpeed_S
{
	ONVIF_Vector2D_S	PanTilt;
	ONVIF_Vector1D_S	Zoom;
}ONVIF_PTZSpeed_S;		//PTZSpeed

typedef struct _ONVIF_PTZConfig_S
{
	char					ForcePersistence[ONVIF_MAX_TOKEN_LEN];
	char					ConfigurationEntity[ONVIF_MAX_TOKEN_LEN];
	char					NodeToken[ONVIF_MAX_TOKEN_LEN];
	char					DefAbsPTPosSpace[ONVIF_MAX_ADDR_LEN];
	char					DefAbsZoomPosSpace[ONVIF_MAX_ADDR_LEN];
	char					DefRelPTTranSpace[ONVIF_MAX_ADDR_LEN];
	char					DefRelZoomTranSpace[ONVIF_MAX_ADDR_LEN];
	char					DefConPTVelSpace[ONVIF_MAX_ADDR_LEN];
	char					DefConZoomVelSpace[ONVIF_MAX_ADDR_LEN];
	ONVIF_PTZSpeed_S		DefaultPTZSpeed;
	char					DefaultPTZTimeout[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PANTILTLIMITS_S	PanTiltLimits;
	ONVIF_ZOOMLIMITS_S		ZoomLimits;
	int						index;				
	int						returncode;			
}ONVIF_PTZConfig_S;		//PTZConfiguration

typedef struct _ONVIF_Space1DDsp_S
{
	char				URI[ONVIF_MAX_ADDR_LEN];
	ONVIF_FloatRange_S	XRange;
}ONVIF_Space1DDsp_S;	//Space1DDsp

typedef struct _ONVIF_Space2DDsp_S
{
	char				URI[ONVIF_MAX_ADDR_LEN];
	ONVIF_FloatRange_S	XRange;
	ONVIF_FloatRange_S	YRange;
}ONVIF_Space2DDsp_S;	//Space2DDsp

typedef struct _ONVIF_PTZSpaces_S
{
	ONVIF_Space2DDsp_S		AbsolutePanTiltPositionSpace;
	ONVIF_Space1DDsp_S		AbsoluteZoomPositionSpace;
	ONVIF_Space2DDsp_S		RelativePanTiltTranslationSpace;
	ONVIF_Space1DDsp_S		RelativeZoomTranslationSpace;
	ONVIF_Space2DDsp_S		ContinuousPanTiltVelocitySpace;
	ONVIF_Space1DDsp_S		ContinuousZoomVelocitySpace;
	ONVIF_Space1DDsp_S		PanTiltSpeedSpace;
	ONVIF_Space1DDsp_S		ZoomSpeedSpace;
}ONVIF_PTZSpaces_S;		//PTZSpaces

typedef struct _ONVIF_PTZNode_S
{
	char					DeviceEntity[ONVIF_MAX_TOKEN_LEN];
	char					Name[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZSpaces_S		SupportedPTZSpaces;
	int						MaximumNumberOfPresets;
	int                     HomeSupported;
	char					AuxiliaryCommands[ONVIF_MAX_ADDR_LEN];	
	int						index;				
	int						returncode;			
	int						resved[4];
}ONVIF_PTZNode_S;	//PTZNode

typedef struct _ONVIF_PTZConfigOpts_S
{
	char				Token[ONVIF_MAX_TOKEN_LEN];
  	ONVIF_PTZSpaces_S	Spaces;
	char				PTZTimeout[ONVIF_MAX_TOKEN_LEN];
}ONVIF_PTZConfigOpts_S;		//PTZConfigurationOptions


typedef struct _ONVIF_PTZVector_S
{
	ONVIF_Vector2D_S	PanTilt;
	ONVIF_Vector1D_S	Zoom;	
}ONVIF_PTZVector_S;		//PTZVector

typedef struct _ONVIF_AbsoluteMove_S
{
	char				ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZVector_S	Destination;
	ONVIF_PTZSpeed_S	Speed;
}ONVIF_AbsoluteMove_S;		//AbsoluteMove


typedef struct _ONVIF_RelativeMove_S
{
	char				ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZVector_S	Translation;
	ONVIF_PTZSpeed_S	Speed;
}ONVIF_RelativeMove_S;		//RelativeMove

typedef struct _ONVIF_ContinuousMove_S
{
	char				ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZSpeed_S	Velocity;
	char				Timeout[ONVIF_MAX_TOKEN_LEN];
}ONVIF_ContinuousMove_S;		//ContinuousMove

typedef struct _ONVIF_Stop_S
{
	char		ProfileToken[ONVIF_MAX_TOKEN_LEN];
	int 		PanTilt;
	int 		Zoom;
}ONVIF_Stop_S;		//Stop

typedef struct _ONVIF_PTZMoveStatus_S
{
	char		PanTilt[ONVIF_MAX_TOKEN_LEN];	//MoveStatus: IDLE|MOVING|UNKNOWN
	char		Zoom[ONVIF_MAX_TOKEN_LEN];		//MoveStatus: IDLE|MOVING|UNKNOWN
}ONVIF_PTZMoveStatus_S;		//PTZMoveStatus

typedef struct _ONVIF_PTZ_STATE_S
{
	char				    ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZVector_S 		Position;
	ONVIF_PTZMoveStatus_S 	MoveStatus;
	char					Error[ONVIF_MAX_TOKEN_LEN];
	char					UtcTime[ONVIF_MAX_TOKEN_LEN];
}ONVIF_PTZ_STATE_S;		//PTZStatus	

typedef struct _ONVIF_PTZPreset_S
{
	char				token[ONVIF_MAX_TOKEN_LEN];	
	char				Name[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZVector_S	PTZPosition;
}ONVIF_PTZPreset_S;		//PTZPreset

typedef struct _ONVIF_Preset_S
{
	char				ProfileToken[ONVIF_MAX_TOKEN_LEN];
	char				PresetName[ONVIF_MAX_TOKEN_LEN];
	char				PresetToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_PTZSpeed_S	Speed;
	ONVIF_PTZPreset_S	Preset;
	char				Timeout[ONVIF_MAX_TOKEN_LEN];
	int					index;			
	int					returncode;			
}ONVIF_Preset_S;		//Preset

typedef struct _ONVIF_SendAuxCmd_S
{
	char			ProfileToken[ONVIF_MAX_TOKEN_LEN];
	char			AuxiliaryData[ONVIF_MAX_ADDR_LEN];
}ONVIF_SendAuxCmd_S;		//SendAuxiliaryCommand

// Events
typedef struct _ONVIF_PullPointSubscription_S
{
	char			Filter[ONVIF_MAX_TOKEN_LEN];				//type:FilterType
	char			InitialTerminationTime[ONVIF_MAX_TOKEN_LEN];//type:AbsoluteOrRelativeTimeType
	char			SubscriptionPolicy[ONVIF_MAX_TOKEN_LEN];
	char			CurrentTime[ONVIF_MAX_TOKEN_LEN];
	char			TerminationTime[ONVIF_MAX_TOKEN_LEN];
}ONVIF_PullPointSubscription_S;		//PullPointSubscription

typedef struct _ONVIF_PullMessages_S
{
	char		Timeout[ONVIF_MAX_TOKEN_LEN];
	char		MessageLimit[ONVIF_MAX_TOKEN_LEN];
	char		CurrentTime[ONVIF_MAX_TOKEN_LEN];
	char		TerminationTime[ONVIF_MAX_TOKEN_LEN];
	char		NotificationMessage[ONVIF_MAX_TOKEN_LEN];
	char		MaxTimeout[ONVIF_MAX_TOKEN_LEN];
	char		MaxMessageLimit[ONVIF_MAX_TOKEN_LEN];

}ONVIF_PullMessages_S;		//PullMessages

typedef struct _ONVIF_EventProperties_S
{
	char		TopicNamespaceLocation[ONVIF_MAX_ADDR_LEN];
	char		FixedTopicSet[ONVIF_MAX_TOKEN_LEN];
	char		TopicSet[ONVIF_MAX_TOKEN_LEN];
	char		TopicExpressionDialect[ONVIF_MAX_TOKEN_LEN];
	char		MessageContentFilterDialect[ONVIF_MAX_ADDR_LEN];
	char		ProducerPropertiesFilterDialect[ONVIF_MAX_ADDR_LEN];
	char		MessageContentSchemaLocation[ONVIF_MAX_ADDR_LEN];
}ONVIF_EventProperties_S;		//EventProperties

//ImagingSettings
//--------------- Media --> Video---------------------
typedef struct _ONVIF_VideoResolution_S
{
	int		Width;
	int		Height;
}ONVIF_VideoResolution_S;	//VideoResolution

typedef struct _ONVIF_VideoSource_S
{
	char					DeviceEntity[ONVIF_MAX_TOKEN_LEN];
	int						Framerate;
	ONVIF_VideoResolution_S	Resolution;
	ONVIF_ImagingSettings_S	Imaging;
}ONVIF_VideoSource_S;	//VideoSource

typedef struct _ONVIF_IntRange_S
{
	int		Min;
	int		Max;
}ONVIF_IntRange_S;

typedef struct _ONVIF_IntRectangleRange_S
{
	ONVIF_IntRange_S	XRange;
	ONVIF_IntRange_S	YRange;
	ONVIF_IntRange_S	WidthRange;
	ONVIF_IntRange_S	HeightRange;
}ONVIF_IntRectangleRange_S;

typedef struct _ONVIF_VideoSourceCFGOp_S
{
	char						ConfigurationToken[ONVIF_MAX_TOKEN_LEN];
	char						ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_IntRectangleRange_S	BoundsRange;
	char						VideoSourceTokensAvailable[ONVIF_MAX_TOKEN_LEN];
}ONVIF_VideoSourceCFGOp_S;	//VideoSourceConfigurationOptions

typedef struct _ONVIF_AudioSource_S
{
	char		DeviceEntity[ONVIF_MAX_TOKEN_LEN];
	int 		Channels;
}ONVIF_AudioSource_S;	//AudioSource

typedef struct _ONVIF_AI_CONFIG_OP_S
{
	char		ConfigurationToken[ONVIF_MAX_TOKEN_LEN];
	char		ProfileToken[ONVIF_MAX_TOKEN_LEN];
	char		InputTokensAvailable[ONVIF_MAX_TOKEN_LEN];
}ONVIF_AI_CONFIG_OP_S;	//AudioSourceConfigurationOptions

typedef struct _ONVIF_Options_S
{
	char		Encoding[ONVIF_MAX_TOKEN_LEN]; 	//G711|G726|AAC
	int			BitrateList;
	int			SampleRateList;
}ONVIF_Options_S;

typedef struct _ONVIF_AE_Options_S
{
	char			ConfigurationToken[ONVIF_MAX_TOKEN_LEN];
	char			ProfileToken[ONVIF_MAX_TOKEN_LEN];
	ONVIF_Options_S	Options;
}ONVIF_AE_Options_S;	//AudioEncoderConfigurationOption

typedef struct  _ONVIF_SUBSCRIBE
{
    int         index;
    char        dstaddr[ONVIF_MAX_ADDR_LEN];    // ip addrees and port
    char        lastime[ONVIF_MAX_TOKEN_LEN];   // last time
    char        srcaddr[ONVIF_MAX_ADDR_LEN];    // local onvif address
    char        curtime[ONVIF_MAX_TOKEN_LEN];   // current time
    char        tertime[ONVIF_MAX_TOKEN_LEN];   // terminate time
    char        subsindex[128];
}ONVIF_SUBSCRIBE_S;

typedef struct  _ONVIF_UNSUBSCRIBE
{
    int         status;                         // result: 0-fail, 1-success
    int         index;
    char        dstaddr[ONVIF_MAX_ADDR_LEN];    // cancel ip address
}ONVIF_UNSUBSCRIBE_S;

typedef struct  _ONVIF_EVENT_SUBSCRIBE
{
    char        index;
    char        status;                       // status: 0-no subscribe user,1 has subscribe user
    char        falsetms;                     //fail times,if fail 60 times,cancel subscribe
    char        recv;
    char        name[ONVIF_MAX_TOKEN_LEN];    // not use now
    char        ipaddr[ONVIF_MAX_ADDR_LEN];   // ip address
    int         port;                         // port
    struct tm   curtm;                        // current time
    struct tm   endtm;                        // end time
    char        subsindex[128];               // reserve
}ONVIF_EVENT_SUBSCRIBE_S;

typedef struct _ONVIF_OSDTime_S
{
	unsigned int 	show;	//0: close  1: open
	unsigned short	x;		//x£¬y£ºdisplay position,range(0~704)(0~576)
	unsigned short	y;      //independent to image resolution,x must multiple to 4

}ONVIF_OSDTime_S;

typedef struct _ONVIF_OSDTitle_S
{
	unsigned int show;		//0: close  1: open
	float		 x;			//x£¬y£ºdisplay position,range(0~704)(0~576)
	float		 y;         //independent to image resolution,x must multiple to 4
	char    	 title[64];	

}ONVIF_OSDTitle_S;

typedef struct _ONVIF_3DZOOM_S
{
	short				startx;							
	short				starty;									
	short				endx;									
	short				endy;									
}ONVIF_3DZOOM_S;

typedef struct _ONVIF_AUX_S
{
	int				rainbrushenable;			
	int				lightenable;		
	
}ONVIF_AUX_S;

typedef struct _ONVIF_ContinuousFocus_S
{
	float	Speed;

}ONVIF_ContinuousFocus_S;	//ContinuousFocus

typedef struct _ONVIF_preset_tour
{
	int	tourenable;		
	
}ONVIF_preset_tour_S;

typedef struct GF_SET_PRESET	//sizeof = 40
{
	char   szTitle[32];			   //title
	unsigned char  nStatus;		   //0: not set  1: set
	unsigned char  nUserPresetNo;  //user preset numer
	unsigned char  iTilt;		   // -3 ~ 90
	unsigned char  iZoom;		   //  0 ~ 20
	unsigned short iPan;		   //  0 ~ 360
	char  devicetype;	           //0--IPC  1--IPC
	char  chn;
} GF_SET_PRESET;

typedef struct ONVIF_AUDIO_OUTPUT_S
{
	int  nVol;
	char name[64];
	char chn;
	
}ONVIF_AUDIO_OUTPUT_S;		
*/

// system time
typedef struct
{
	int hour;
	int minute;
	int second;
	int year;
	int month;
	int day;

}onvif_date_time;

typedef struct
{
    char tTimeZone[20];
	onvif_date_time tUtcTime;
    onvif_date_time tLocalTime;
}onvif_system_date_time;

// default gateway
typedef struct
{
    char ifName[64];
    unsigned int gateWay;
    unsigned int srcAddr;
    unsigned int dstAddr;
}route_info;


#endif

