#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"

#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
//#include "RM68200_720x1280.h"
//#include "SAT070AT50_800x480.h"
#include MI_DISP_INTF_LCD_HEADER
//#include "FPGA_800x480_60.h"
//#include "LX50FWB4001_480x854.h"
//#include "spi_cmd_480x854_v2.h"
#if defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#define MAKE_YUYV_VALUE(y,u,v)    ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK                MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE                MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                  MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN                MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE                 MAKE_YUYV_VALUE(29,225,107)

#ifdef MI_DISP_INTF_TYPE_HDMI
#define HDMI_TIMING(x) x
#else
#define HDMI_TIMING(x) 0
#endif

stTimingArray_t astTimingArray[] = {
    {
        .desc = "user",
        .eOutputTiming = E_MI_DISP_OUTPUT_USER,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_MAX),
    },
    {
        .desc = "480p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_480P60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_480_60P),
        .u16Width = 640,
        .u16Height = 480
    },
    {
        .desc = "576p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_576P50,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_576_50P),
        .u16Width = 720,
        .u16Height = 576
    },
    {
        .desc = "720p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_720P50,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_720_50P),
        .u16Width = 1280,
        .u16Height = 720
    },
    {
        .desc = "720p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_720P60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_720_60P),
        .u16Width = 1280,
        .u16Height = 720
    },
    {
        .desc = "1024x768_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1024x768_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1024x768_60P),
        .u16Width = 1024,
        .u16Height = 768
    },
    {
        .desc = "1080p24",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P24,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1080_24P),
        .u16Width = 1920,
        .u16Height = 1080
    },
    {
        .desc = "1080p25",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P25,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1080_25P),
        .u16Width = 1920,
        .u16Height = 1080
    },
    {
        .desc = "1080p30",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P30,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1080_30P),
        .u16Width = 1920,
        .u16Height = 1080
    },
    {
        .desc = "1080p50",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P50,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1080_50P),
        .u16Width = 1920,
        .u16Height = 1080
    },
    {
        .desc = "1080p60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1080P60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1080_60P),
        .u16Width = 1920,
        .u16Height = 1080
    },
    {
        .desc = "1280x800_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1280x800_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1280x800_60P),
        .u16Width = 1280,
        .u16Height = 800
    },
    {
        .desc = "1280x1024_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1280x1024_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1280x1024_60P),
        .u16Width = 1280,
        .u16Height = 1024
    },
    {
        .desc = "1366x768_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1366x768_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1366x768_60P), //HDMI don't support this timing
        .u16Width = 1366,
        .u16Height = 768
    },
    {
        .desc = "1440x900_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1440x900_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1440x900_60P),
        .u16Width = 1440,
        .u16Height = 900
    },
    {
        .desc = "1680x1050_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1680x1050_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1680x1050_60P), //HDMI don't support this timing
        .u16Width = 1680,
        .u16Height = 1050
    },
    {
        .desc = "1600x1200_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_1600x1200_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_1600x1200_60P),
        .u16Width = 1600,
        .u16Height = 1200
    },
    {
        .desc = "2560x1440_30",
        .eOutputTiming = E_MI_DISP_OUTPUT_2560x1440_30,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_MAX), //HDMI don't support this timing
        .u16Width = 2560,
        .u16Height = 1440
    },
    {
        .desc = "2560x1440_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_MAX, //not defined
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_MAX),   //HDMI don't support this timing
        .u16Width = 2560,
        .u16Height = 1440
    },
    {
        .desc = "2560x1600_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_2560x1600_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_MAX), //HDMI don't support this timing
        .u16Width = 2560,
        .u16Height = 1600
    },
    {
        .desc = "3840x2160_30",
        .eOutputTiming = E_MI_DISP_OUTPUT_3840x2160_30,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_4K2K_30P),
        .u16Width = 3840,
        .u16Height = 2160
    },
    {
        .desc = "3840x2160_60",
        .eOutputTiming = E_MI_DISP_OUTPUT_3840x2160_60,
        .eHdmiTiming = HDMI_TIMING(E_MI_HDMI_TIMING_4K2K_60P),
        .u16Width = 3840,
        .u16Height = 2160
    },
};
#ifdef MI_DISP_INTF_TYPE_HDMI
static MI_S32 Hdmi_Start(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_TimingType_e eTimingType)
{
    MI_HDMI_Attr_t stAttr;
    memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));

    if(eTimingType == HDMI_TIMING(E_MI_HDMI_TIMING_MAX)) {
        printf("[%s][%d]unsupported hdmi timing %d,reset to 1080p60\n", __FUNCTION__, __LINE__, eTimingType);
        eTimingType = HDMI_TIMING(E_MI_HDMI_TIMING_1080_60P);
    }

    stAttr.stEnInfoFrame.bEnableAudInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableAviInfoFrame = FALSE;
    stAttr.stEnInfoFrame.bEnableSpdInfoFrame = FALSE;
    stAttr.stAudioAttr.bEnableAudio = TRUE;
    stAttr.stAudioAttr.bIsMultiChannel = 0;
    stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
    stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
    stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
    stAttr.stVideoAttr.bEnableVideo = TRUE;
    stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_YCBCR444; //default color type
    stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
    stAttr.stVideoAttr.eTimingType = eTimingType;
    stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
    MI_HDMI_SetAttr(eHdmi, &stAttr);
    MI_HDMI_Start(eHdmi);
    return MI_SUCCESS;
}
#endif

static stTimingArray_t *gettiming(MI_DISP_OutputTiming_e eOutputTiming)
{
    int index = 0;

    for(index = 0; index < sizeof(astTimingArray); index++) {
        if(eOutputTiming == astTimingArray[index].eOutputTiming) {
            return astTimingArray + index;
        }
    }

    return NULL;
}
stTimingArray_t *getTimingByAlias(char *sTimingAlias)
{
    int index = 0;

    for(index = 0; index < sizeof(astTimingArray); index++) {
        if(0 == strcasecmp(sTimingAlias, astTimingArray[index].desc))
            return astTimingArray + index;
    }

    return NULL;
}
void dumpTimingAlias(void)
{
    int index = 0;

    for(index = 0; index < sizeof(astTimingArray); index++) {
        fprintf(stderr, "%s\n", astTimingArray[index].desc);
    }
}


int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    stTimingArray_t *pstTiming = gettiming(pstDispPubAttr->eIntfSync);
    MI_SYS_Init();

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    stInputPortAttr.u16SrcWidth = pstTiming->u16Width;
    stInputPortAttr.u16SrcHeight = pstTiming->u16Height;
    stInputPortAttr.stDispWin.u16X = 0;
    stInputPortAttr.stDispWin.u16Y = 0;
    stInputPortAttr.stDispWin.u16Width = pstTiming->u16Width;
    stInputPortAttr.stDispWin.u16Height = pstTiming->u16Height;

    if(pstDispPubAttr->eIntfType == E_MI_DISP_INTF_HDMI) {
#ifdef MI_DISP_INTF_TYPE_HDMI
        MI_HDMI_InitParam_t stInitParam;
        memset(&stInitParam, 0, sizeof(MI_HDMI_InitParam_t));
        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = NULL;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);
        Hdmi_Start(E_MI_HDMI_ID_0, pstTiming->eHdmiTiming);
#endif
    } else if(pstDispPubAttr->eIntfType == E_MI_DISP_INTF_VGA) {
        eLinkType = E_MI_PNL_LINK_DAC_P;
    } else if(pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD) {
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
        eLinkType = E_MI_PNL_LINK_TTL;
        stInputPortAttr.u16SrcWidth = stPanelParam.u16Width;
        stInputPortAttr.u16SrcHeight = stPanelParam.u16Height;
        stInputPortAttr.stDispWin.u16X = 0;
        stInputPortAttr.stDispWin.u16Y = 0;
        stInputPortAttr.stDispWin.u16Width = stPanelParam.u16Width;
        stInputPortAttr.stDispWin.u16Height = stPanelParam.u16Height;
    }

    MI_DISP_SetPubAttr(0, pstDispPubAttr);
    MI_DISP_Enable(0);
    //MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    //MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    if(pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD) {
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);

        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI) {
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
    }

    return 0;
}
int sstar_disp_Deinit(MI_DISP_PubAttr_t *pstDispPubAttr)
{

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);

    switch(pstDispPubAttr->eIntfType) {
#ifdef MI_DISP_INTF_TYPE_HDMI
        case E_MI_DISP_INTF_HDMI:
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

    MI_SYS_Exit();
}


#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

