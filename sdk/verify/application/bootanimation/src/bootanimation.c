#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
//#include "mi_panel_datatype.h"
//#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"

//#include "SAT070CP50_1024x600.h"
#include "CC0702I50R_1024x600.h"


#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define VDEC_CHN_ID     0

#define VDEC_INPUT_WIDTH     1920
#define VDEC_INPUT_HEIGHT    1080

#if ENABLE_HDMI
    #define VDEC_OUTPUT_WIDTH     1920
    #define VDEC_OUTPUT_HEIGHT    1080
#else
    #define VDEC_OUTPUT_WIDTH     1024
    #define VDEC_OUTPUT_HEIGHT    600
#endif

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)

#define ENABLE_ROTATE   1
#define NALU_PACKET_SIZE            512*1024
#define MI_U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])

  

static int inVdecWidth,inVdecHeight,OutVdecWidth,OutVdecHeight;
static int inDispWidth,inDispHeight,OutX,OutY,OutDispWidth,OutDispHeight;

static int bRota;

int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_InputPortAttr_t stInputPortAttr;
#if ENABLE_HDMI
    MI_HDMI_InitParam_t stInitParam;
    MI_HDMI_Attr_t stAttr;
#endif
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    MI_SYS_Init();

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        pstDispPubAttr->stSyncInfo.u16Vact = stPanelParam.u16Height;
        pstDispPubAttr->stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
                                                                      stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Hact = stPanelParam.u16Width;
        pstDispPubAttr->stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
                                                                      stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Bvact = 0;
        pstDispPubAttr->stSyncInfo.u16Bvbb = 0;
        pstDispPubAttr->stSyncInfo.u16Bvfb = 0;
        pstDispPubAttr->stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        pstDispPubAttr->stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        pstDispPubAttr->stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_USER;
        pstDispPubAttr->eIntfType = E_MI_DISP_INTF_LCD;
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
#if USE_MIPI
        eLinkType = E_MI_PNL_LINK_MIPI_DSI;
#else
        eLinkType = E_MI_PNL_LINK_TTL;
#endif
        stInputPortAttr.u16SrcWidth = inDispWidth;
        stInputPortAttr.u16SrcHeight = inDispHeight;
        stInputPortAttr.stDispWin.u16X = OutX;
        stInputPortAttr.stDispWin.u16Y = OutY;
        if(bRota)
        {
            stInputPortAttr.stDispWin.u16Width = OutDispHeight;
            stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stInputPortAttr.stDispWin.u16Width = OutDispWidth;
            stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
        MI_DISP_Disable(0);
        MI_DISP_DisableInputPort(0, 0);
        MI_DISP_SetPubAttr(0, pstDispPubAttr);

        //MI_DISP_Enable(0);
        
        MI_DISP_BindVideoLayer(0, 0);
        memset(&stLayerAttr, 0, sizeof(stLayerAttr));
        if(bRota)
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerSize.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerSize.u16Height = OutDispHeight;
        }
        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerDispWin.u16X      = OutX;
        stLayerAttr.stVidLayerDispWin.u16Y      = OutY;
        if(bRota)
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispHeight;
        }
        MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
        MI_DISP_EnableVideoLayer(0);

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        //MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);	
        usleep(30*1000);
        MI_DISP_Enable(0);
        MI_DISP_EnableInputPort(0, 0);
    }
#if ENABLE_HDMI
    else if (E_MI_DISP_INTF_HDMI == pstDispPubAttr->eIntfType)
    {
        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = Hdmi_callback_impl;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
        stAttr.stAudioAttr.bEnableAudio = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel = 0;
        stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo = TRUE;
        stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type

#if ENABLE_HDMI_4K
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_24BIT;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
#else
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
        stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
#endif
        stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
#if ENABLE_HDMI_4K
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_3840x2160_30;
#else
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_1080P60;
#endif
        MI_DISP_SetPubAttr(0, pstDispPubAttr);

        MI_DISP_Enable(0);
        MI_DISP_BindVideoLayer(0, 0);
        memset(&stLayerAttr, 0, sizeof(stLayerAttr));
        
#if ENABLE_HDMI_4K
        if(bRota)
        {
            stLayerAttr.stVidLayerSize.u16Width  = 2160;
            stLayerAttr.stVidLayerSize.u16Height = 3840;
        }
        else
        {
            stLayerAttr.stVidLayerSize.u16Width  = 3840;
            stLayerAttr.stVidLayerSize.u16Height = 2160;
        }
#else
        if(bRota)
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerSize.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerSize.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerSize.u16Height = OutDispHeight;
        }

#endif
        
        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerDispWin.u16X      = OutX;
        stLayerAttr.stVidLayerDispWin.u16Y      = OutY;
        
#if ENABLE_HDMI_4K
        if(bRota)
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = 2160;
            stLayerAttr.stVidLayerDispWin.u16Height = 3840;
        }
        else
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = 3840;
            stLayerAttr.stVidLayerDispWin.u16Height = 2160;
        }
#else
        if(bRota)
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispHeight;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stLayerAttr.stVidLayerDispWin.u16Width  = OutDispWidth;
            stLayerAttr.stVidLayerDispWin.u16Height = OutDispHeight;
        }
#endif
      
        MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
        MI_DISP_EnableVideoLayer(0);

        stInputPortAttr.u16SrcWidth = inDispWidth;
        stInputPortAttr.u16SrcHeight = inDispHeight;
        stInputPortAttr.stDispWin.u16X = OutX;
        stInputPortAttr.stDispWin.u16Y = OutY;
#if ENABLE_HDMI_4K
        if(bRota)
        {
            stInputPortAttr.stDispWin.u16Width = OutDispHeight;
            stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stInputPortAttr.stDispWin.u16Width = OutDispWidth;
            stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#else
        if(bRota)
        {
            stInputPortAttr.stDispWin.u16Width = OutDispHeight;
            stInputPortAttr.stDispWin.u16Height = OutDispWidth;
        }
        else
        {
            stInputPortAttr.stDispWin.u16Width = OutDispWidth;
            stInputPortAttr.stDispWin.u16Height = OutDispHeight;
        }
#endif

        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }
#endif
#if 0
    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        MI_DISP_Disable(0);
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
#if USE_MIPI
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
#endif
        }
		MI_DISP_Enable(0);
    }
#endif
    return 0;
}
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr)
{

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);
#if 0
    switch(pstDispPubAttr->eIntfType) {
#if ENABLE_HDMI
        case E_MI_DISP_INTF_HDMI:
            MI_HDMI_Stop(E_MI_HDMI_ID_0);
            MI_HDMI_Close(E_MI_HDMI_ID_0);
            MI_HDMI_DeInit();
            break;
#endif
        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
        default:
            MI_PANEL_DeInit();

    }
#endif
    MI_SYS_Exit();
    printf("sstar_disp_Deinit...\n");

    return 0;
}

int sstar_vdec_init(void)
{
    //init vdec
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
	MI_VDEC_InitParam_t stVdecInitParam;
	
	memset(&stVdecInitParam, 0, sizeof(MI_VDEC_InitParam_t));

    stVdecInitParam.bDisableLowLatency = true;

    //STCHECKRESULT(MI_VDEC_InitDev(&stVdecInitParam));

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType   = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;
    stVdecChnAttr.eVideoMode   = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize   = 1 * 1920 * 1080;
    stVdecChnAttr.u32PicWidth  = inVdecWidth;
    stVdecChnAttr.u32PicHeight = inVdecHeight;
    stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority  = 0;

    STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));

    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width  = OutVdecWidth;
    stOutputPortAttr.u16Height = OutVdecHeight;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(0, &stOutputPortAttr));

    MI_SYS_ChnPort_t  stChnPort;
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = stVdecChn;
    stChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));

    //bind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 30, 30));

    if(bRota)
    {
        MI_DISP_RotateConfig_t stRotateConfig;
        memset(&stRotateConfig, 0, sizeof(MI_DISP_RotateConfig_t));
        stRotateConfig.eRotateMode      = E_MI_DISP_ROTATE_90;

        MI_DISP_SetVideoLayerRotateMode(0, &stRotateConfig);
    }

    return 0;
}

int sstar_vdec_deInit(void)
{
    //Unbind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));

    STCHECKRESULT(MI_VDEC_StopChn(0));
    STCHECKRESULT(MI_VDEC_DestroyChn(0));

    return 0;
}

int main (int argc, char **argv)
{
    const char *filename;
    MI_U8 au8Header[32] = {0};
    FILE *readfp = NULL;
    MI_U32 u32Pos = 0;
    MI_U8 *pu8Buf = NULL;
    MI_S32 s32Len = 0;
    MI_U32 u32FrameLen = 0;
    MI_U32 u32FpBackLen = 0;
    MI_VDEC_VideoStream_t stVdecStream;
    MI_VDEC_ChnStat_t pstChnStat;

    filename    = argv[1];
    inVdecWidth = atoi(argv[2]);
    inVdecHeight = atoi(argv[3]);
    OutVdecWidth = atoi(argv[4]);
    OutVdecHeight = atoi(argv[5]);

    inDispWidth = atoi(argv[6]);
    inDispHeight = atoi(argv[7]);
    OutX = atoi(argv[8]);
    OutY = atoi(argv[9]);
    OutDispWidth = atoi(argv[10]);
    OutDispHeight = atoi(argv[11]);
    
    bRota = atoi(argv[12]);
    

    MI_DISP_PubAttr_t stDispPubAttr;
#if ENABLE_HDMI
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
#else
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
#endif
    sstar_disp_init(&stDispPubAttr);

    //init SDK
    sstar_vdec_init();

    readfp = fopen(filename, "rb"); //ES
    if (!readfp)
    {
        printf("Open %s failed!\n", filename);
        return -1;
    }
    memset(au8Header, 0, 16);
    fseek(readfp, 0, SEEK_CUR);
    
    pu8Buf = malloc(NALU_PACKET_SIZE);
    if(!pu8Buf)
    {
        printf("malloc buf fail\n");
        return -1;
    }
    
    while (fread(au8Header, 1, 16, readfp))
    {
        u32FrameLen = MI_U32VALUE(au8Header, 4);
        if(u32FrameLen > NALU_PACKET_SIZE)
        {
            break;
        }
        s32Len = fread(pu8Buf, 1, u32FrameLen, readfp);
        if(s32Len <= 0)
        {
            break;
        }

        stVdecStream.pu8Addr = pu8Buf;
        stVdecStream.u32Len = s32Len;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len + 16; //back length

        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && (stVdecStream.pu8Addr[4] & 0xf == 0x5) || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4])
        {
            usleep(30 * 1000);
        }
        
        if (MI_SUCCESS !=  MI_VDEC_SendStream(VDEC_CHN_ID, &stVdecStream, 20))
        {
            fseek(readfp, - u32FpBackLen, SEEK_CUR);
        }

        memset(au8Header, 0, 16);

    }
    
    while(1)
    {
        MI_VDEC_GetChnStat(VDEC_CHN_ID,&pstChnStat);
        if(pstChnStat.u32LeftStreamBytes == 0)
            break;
        usleep(1 * 1000);
    }
    
    free(pu8Buf);
    sstar_vdec_deInit();
    sstar_disp_Deinit(&stDispPubAttr);
    return 0;
}
