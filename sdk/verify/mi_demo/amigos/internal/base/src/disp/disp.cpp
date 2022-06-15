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

#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>

#include "mi_sys.h"
#include "mi_panel.h"
#include "mi_disp.h"


#include "SAT070CP50_1024x600.h"

#include "disp.h"


Disp::Disp()
{
}
Disp::~Disp()
{
}
void Disp::Init()
{
    MI_U8 u8LayerId = 0;
    MI_U32 u32PortOut_x = 0;
    MI_U32 u32PortOut_y = 0;

    MI_PANEL_LinkType_e eLinkType;
    
    stDispInfo_t stDispInfo;
    MI_DISP_PubAttr_t stPubAttr;
    
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_RotateConfig_t stRotateConfig;
    

    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;

    //get para
    stDispInfo.intDisplayWidth = GetIniInt(stModDesc.modKeyString, "DISP_W");;
    stDispInfo.intDisplayHeight = GetIniInt(stModDesc.modKeyString, "DISP_H");;
    stDispInfo.intLinkType = GetIniInt(stModDesc.modKeyString, "Link_TYPE");;
    stDispInfo.intBackGroundColor = GetIniInt(stModDesc.modKeyString, "BK_COLOR");


    //pub attr
    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    
    if(stPubAttr.eIntfType == E_MI_DISP_INTF_LCD)
    {
        stPubAttr.stSyncInfo.u16Vact = stPanelParam.u16Height;
        stPubAttr.stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        stPubAttr.stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth + stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        stPubAttr.stSyncInfo.u16Hact = stPanelParam.u16Width;
        stPubAttr.stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        stPubAttr.stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth + stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        stPubAttr.stSyncInfo.u16Bvact = 0;
        stPubAttr.stSyncInfo.u16Bvbb = 0;
        stPubAttr.stSyncInfo.u16Bvfb = 0;
        stPubAttr.stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        stPubAttr.stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        stPubAttr.stSyncInfo.u32FrameRate = stPanelParam.u16DCLK*1000000/(stPanelParam.u16HTotal*stPanelParam.u16VTotal);
    }

    switch(stDispInfo.intBackGroundColor)
    {
        case 0:
            stPubAttr.u32BgColor = YUYV_BLACK;
            break;
        case 1:
            stPubAttr.u32BgColor = YUYV_WHITE;
            break;
        case 2:
            stPubAttr.u32BgColor = YUYV_RED;
            break;
        case 3:
            stPubAttr.u32BgColor = YUYV_GREEN;
            break;
        case 4:
            stPubAttr.u32BgColor = YUYV_BLUE;
            break;
        default:
            stPubAttr.u32BgColor = YUYV_BLACK;
            break;
    }

    //set disp pub
    MI_DISP_SetPubAttr((MI_DISP_DEV)stModDesc.devId,  &stPubAttr);
    MI_DISP_Enable((MI_DISP_DEV)stModDesc.devId);


    //set layer
    MI_DISP_BindVideoLayer((MI_DISP_LAYER)u8LayerId, (MI_DISP_DEV)stModDesc.devId);
    MI_DISP_EnableVideoLayer((MI_DISP_LAYER)u8LayerId);

    //set inputport
    for (itMapIn = mapModInputInfo.begin(); itMapIn != mapModInputInfo.end(); ++itMapIn)
    {
        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        u32PortOut_x = (stPanelParam.u16Width - stDispInfo.intDisplayWidth)/2;
        u32PortOut_y = (stPanelParam.u16Height - stDispInfo.intDisplayHeight)/2;

        stInputPortAttr.u16SrcWidth = stDispInfo.intDisplayWidth;
        stInputPortAttr.u16SrcHeight = stDispInfo.intDisplayHeight;
        stInputPortAttr.stDispWin.u16X = u32PortOut_x;
        stInputPortAttr.stDispWin.u16Y = u32PortOut_y;
        stInputPortAttr.stDispWin.u16Width = stDispInfo.intDisplayWidth;
        stInputPortAttr.stDispWin.u16Height = stDispInfo.intDisplayHeight;
        MI_DISP_SetInputPortAttr((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapIn->second.curPortId, &stInputPortAttr);


        //rotate
        stRotateConfig.eRotateMode = E_MI_DISP_ROTATE_NONE;
        MI_DISP_SetVideoLayerRotateMode((MI_DISP_LAYER)u8LayerId, &stRotateConfig);

        //enable inputport
        MI_DISP_EnableInputPort((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapIn->second.curPortId);
        MI_DISP_SetInputPortSyncMode((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapIn->second.curPortId, E_MI_DISP_SYNC_MODE_FREE_RUN);
    }

    //set panel config
    if(stDispInfo.intLinkType == 0)
    {
        eLinkType = E_MI_PNL_LINK_TTL;
    }
    else if(stDispInfo.intLinkType == 11)
    {
        eLinkType = E_MI_PNL_LINK_MIPI_DSI;
    }
    
    if(stPubAttr.eIntfType == E_MI_DISP_INTF_LCD 
        && (eLinkType == E_MI_PNL_LINK_TTL || eLinkType == E_MI_PNL_LINK_MIPI_DSI))
    {
#if INTERFACE_PANEL
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
#endif
    }
}

void Disp::Deinit()
{
    MI_U8 u8LayerId = 0;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;

    for (itMapIn = mapModInputInfo.begin(); itMapIn != mapModInputInfo.end(); ++itMapIn)
    {
        MI_DISP_DisableInputPort(u8LayerId, itMapIn->second.curPortId);
    }
    MI_DISP_DisableVideoLayer(u8LayerId);
    MI_DISP_UnBindVideoLayer(u8LayerId, (MI_DISP_DEV)stModDesc.devId);

    MI_DISP_Disable((MI_DISP_DEV)stModDesc.devId);
    
    MI_PANEL_DeInit();
}