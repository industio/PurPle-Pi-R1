#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"

#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"


#define UI_1024_600 0
#define USE_MIPI    1


#if UI_1024_600
//#include "SAT070CP50_1024x600.h"
#include "CC0702I50R_1024x600.h"
#else
#if USE_MIPI
//#include "EK79007_1024x600_MIPI.h"
#include "EQT700BKJ004P_1024x600_MIPI.h"
#else
//#include "SAT070AT50_800x480.h"
#include "CC021I40R2_480x480.h"
#endif
#endif

// loacl play res
#if UI_1024_600
//#define LOCAL_VIDEO_W  822
//#define LOCAL_VIDEO_H  464
//#define LOCAL_VIDEO_X  100
//#define LOCAL_VIDEO_Y  60
#define LOCAL_VIDEO_W  1024
#define LOCAL_VIDEO_H  600
#define LOCAL_VIDEO_X  0
#define LOCAL_VIDEO_Y  0
#else
//#define LOCAL_VIDEO_W  640
//#define LOCAL_VIDEO_H  360
//#define LOCAL_VIDEO_X  100
//#define LOCAL_VIDEO_Y  60
#define LOCAL_VIDEO_W  1024
#define LOCAL_VIDEO_H  600
#define LOCAL_VIDEO_X  0
#define LOCAL_VIDEO_Y  0
#endif

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif
int sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_InputPortAttr_t stInputPortAttr;

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
    }

    stInputPortAttr.u16SrcWidth = LOCAL_VIDEO_W;
    stInputPortAttr.u16SrcHeight = LOCAL_VIDEO_H;
    stInputPortAttr.stDispWin.u16X = LOCAL_VIDEO_X;
    stInputPortAttr.stDispWin.u16Y = LOCAL_VIDEO_Y;
    stInputPortAttr.stDispWin.u16Width = LOCAL_VIDEO_W;
    stInputPortAttr.stDispWin.u16Height = LOCAL_VIDEO_H;
    printf("DISP width: %d,height: %d\n",stInputPortAttr.stDispWin.u16Width,stInputPortAttr.stDispWin.u16Height);
    MI_DISP_SetPubAttr(0, pstDispPubAttr);
    MI_DISP_Enable(0);
    MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
#if USE_MIPI
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
#endif
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
        case E_MI_DISP_INTF_HDMI:
            break;

        case E_MI_DISP_INTF_VGA:
            break;

        case E_MI_DISP_INTF_LCD:
        default:
            MI_PANEL_DeInit();

    }

    MI_SYS_Exit();
    printf("sstar_disp_Deinit...\n");

    return 0;
}

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

