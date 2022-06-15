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

#include "mapi_syscfg_table.h"

typedef unsigned char MS_U8;

#define BOARD_COMMON_NULL       0x00

#define SOFTWARE_VERSION      "0001"
//**********************************************************************
//** System-TvParam
//**********************************************************************
#define BOARD_TV_PRODUCT_TYPE       E_DTV_Only
#define BOARD_ATV_SYSTEM_TYPE       E_NTSC_ENABLE

#define HDMI_FOR_MHL_PORT   MAPI_INPUT_SOURCE_HDMI
#define STB_TUNER_T2



//For Example:(DVBT_ENABLE|DVBC_ENABLE|DVBS_ENABLE |DVBT2_ENABLE|DVBS2_ENABLE|DTMB_ENABLE|ATSC_ENABLE|ISDB_ENABLE)


#define BOARD_AUDIO_SYSTEM_TYPE     E_BTSC_ENABLE
#define BOARD_STB_SYSTEM_TYPE       E_STB_ENABLE

//For Example:(_FBL_ENABLE|_PWS_ENABLE| _AUDIO_SIF_ENABLE | _PRE_SCALE_DOWN_ENABLE | _CCIR_VEOUT_ENABLE | _SCART_OUT_ENABLE | _MM_FRC_ENABLE | _MARCOVISION_ENABLE )
#define BOARD_IP_ENABLE                         _MM_FRC_ENABLE


#define BOARD_ROUTE_PATH_1              E_ROUTE_DVBT2



#define BOARD_ROUTE_PATH_2              E_ROUTE_DVBT2

#define BOARD_ROUTE_PATH_3              BOARD_COMMON_NULL
#define BOARD_ROUTE_PATH_4              BOARD_COMMON_NULL

//CURRENT MODE Setting
#define BOARD_DAC_CURRENT_MODE         E_CURRENT_MODE_FULL


#define BOARD_SAW_TYPE NO_SAW
#define BOARD_SAR_CHANNEL E_SAR_NC

//**********************************************************************
//** Video relative setting
//**********************************************************************

//EN_Mode_1135,
//EN_Mode_1135_1P5
#define BOARD_VD_CAP_WIN_MODE     EN_Mode_1135
#define D_BOARD_PWM_CH 1

//**********************************************************************
//** Panel relative setting
//**********************************************************************

#define BOARD_LVDS_CONNECT_TYPE        0x00


#define PANEL_PDP_10BIT                 1

#define PANEL_SWAP_LVDS_POL             0

#define PANEL_SWAP_LVDS_CH              0

#define PANEL_CONNECTOR_SWAP_PORT       0

#define LVDS_PN_SWAP_L                  0x00

#define LVDS_PN_SWAP_H                  0x00

//**********************************************************************
//** System-IR
//**********************************************************************

#define BOARD_IR_CRYSTAL_CLK_HZ     12000000

//#define Board_PowerUpKeyCode 0x00
MS_U8 Board_PowerUpKeyCode[32] = {
    0x46, 0xFF, 0xFF, 0xFF,  //SStar IRKEY_POWER : 0x46
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};


//**********************************************************************
//** InputMux
//**********************************************************************

#define BOARD_Demodulator       1       //1: internal, 0:external
//#define BOARD_ATV_InputSrc_Type             E_MAPI_INPUT_SOURCE_INVALID

MS_U8 u8BOARD_DVBT_DSPRegInitExt[]={};
MS_U8 u8BOARD_DMD_DVBT_InitExt[]=
{
#if (BOARD_SAW_TYPE==DUAL_SAW)
// 0:DUAL_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==EXTERNAL_SINGLE_SAW)
// 1 EXTERNAL_SINGLE_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==SILICON_TUNER)
// 2 INTERNAL_SINGLE_SAW_DIF or SILICON_TUNER
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==NO_SAW)
// 3 NO_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==INTERNAL_SINGLE_SAW_VIF)
// 4 INTERNAL_SINGLE_SAW_VIF
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    1, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    1, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==NO_SAW_DIF)
// 5 NO_SAW (DIF)
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#else
#print "BOARD_SAW_TYPE Error"
#endif
};
// DVBC
/*
    E_DMD_DVBC_NO_SIGNAL_GAIN_TH_L,
    E_DMD_DVBC_NO_SIGNAL_GAIN_TH_H,
*/
/*
MS_U8 u8BOARD_DVBC_DSPRegInitExt[]={
                                                                    1, // version, should be matched with library
                                                                    0, // reserved
                                                                    0, // Size_L
                                                                    0, // Size_H
                                                                    E_DMD_DVBC_OP_NORMALIF_EN, // Addr_L
                                                                    E_DMD_DVBC_OP_NORMALIF_EN>>8, // Addr_H
                                                                    0xFF, // Mask
                                                                    0x01}; // Value
*/
MS_U8 u8BOARD_DVBC_DSPRegInitExt[]={};
MS_U8 u8BOARD_DMD_DVBC_InitExt[]=
{
#if (BOARD_SAW_TYPE==DUAL_SAW)
// 0:DUAL_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==EXTERNAL_SINGLE_SAW)
// 1 EXTERNAL_SINGLE_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==SILICON_TUNER)
// 2 INTERNAL_SINGLE_SAW_DIF or SILICON_TUNER
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==NO_SAW)
// 3 NO_SAW
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==INTERNAL_SINGLE_SAW_VIF)
// 4 INTERNAL_SINGLE_SAW_VIF
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    1, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    1, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#elif (BOARD_SAW_TYPE==NO_SAW_DIF)
// 5 NO_SAW (DIF)
    3, // version
    0, // reserved
    0xFF, // TS_CLK
    1, // RFAGC tristate control default value, 1:trisate 0:non-tristate,never modify unless you know the meaning
    (MS_U8)(36167>>24), // IF Frequency
    (MS_U8)(36167>>16),
    (MS_U8)(36167>>8),
    (MS_U8)(36167>>0),
    (MS_U8)(45474>>24), // FS Frequency
    (MS_U8)(45474>>16),
    (MS_U8)(45474>>8),
    (MS_U8)(45474>>0),
    0, // IQ Swap
    0, // u8ADCIQMode : 0=I path, 1=Q path, 2=both IQ
    0, // u8PadSel : 0=Normal, 1=analog pad
    0, // bPGAEnable : 0=disable, 1=enable
    5, // u8PGAGain : default 5
#else
#print "BOARD_SAW_TYPE Error"
#endif
};

//ATSC demod config
MS_U8 u8BOARD_ATSC_DMD_Config[]=
{
    // Locking Time:
    // u16VSBAGCLockCheckTime : default value=50
    (MS_U8)(50>>8),
    (MS_U8)(50>>0),
    // u16VSBPreLockCheckTime : default value=300
    (MS_U8)(300>>8),
    (MS_U8)(300>>0),
    // u16VSBFSyncLockCheckTime : default value=1200
    (MS_U8)(1200>>8),
    (MS_U8)(1200>>0),
    // u16VSBFECLockCheckTime : default value=5000
    (MS_U8)(5000>>8),
    (MS_U8)(5000>>0),
    // u16QAMAGCLockCheckTime : default value=50
    (MS_U8)(50>>8),
    (MS_U8)(50>>0),
    // u16QAMPreLockCheckTime : default value=1000
    (MS_U8)(1000>>8),
    (MS_U8)(1000>>0),
    // u16QAMMainLockCheckTime : default value=3000
    (MS_U8)(3000>>8),
    (MS_U8)(3000>>0),
    // u16AGC_REFERENCE : default value=0
    (MS_U8)(0>>8),
    (MS_U8)(0>>0),

    // By IC:
    // u8IS_DUAL : default value=0
    0,
    // bIsExtDemod : default value=0
    0
};
MS_U8* u8BOARD_DMD_ATSC_InitConf = u8BOARD_ATSC_DMD_Config;

//**********************************************************************
//** WatchDog
//**********************************************************************

#define BOARD_WATCHDOG_ENABLE 			1
#define BOARD_WATCHDOB_TIMER_REG		0x00003008
#define BOARD_WATCHDOB_TIMER				10


#ifdef BOARD_HDMI_EDID_InfoCount
#undef BOARD_HDMI_EDID_InfoCount
#endif
#define BOARD_HDMI_EDID_InfoCount 1

//**********************************************************************
//** HDMI HOT PLUG INVERSE INFO
//** It supports 32 sets of HDMI now
//** Every bit in HOT_PLUG_INVERSE stands for hot plug inverse bit of corresponed HW HDMI port
//** eg: HOT_PLUG_INVERSE = 0xFFFE => INPUT_PORT_DVI0 is the only one that is not inversed.
//**********************************************************************
#define HOT_PLUG_INVERSE          0xFFFF

#if (STB_ENABLE == 1)
#define PANEL_INFO_SIZE     17

static PanelType astPanelCfg[PANEL_INFO_SIZE] =
{
    // Panel Name, PanelDither, PanelLinkType, PanelDualPort, PanelSwapPort,
    // PanelSwapOdd_ML, PanelSwapEven_ML, PanelSwapOdd_RB, PanelSwapEven_RB,
    // PanelSwapLVDS_POL, PanelSwapLVDS_CH, PanelPDP10BIT, PanelLVDS_TI_MODE,
    // PanelDCLKDelay, PanelInvDCLK, PanelInvDE, PanelInvHSync, PanelInvVSync,
    // PanelDCLKCurrent, PanelDECurrent, PanelODDDataCurrent, PanelEvenDataCurrent,
    // PanelOnTiming1, PanelOnTiming2, PanelOffTiming1, PanelOffTiming2, PanelHSyncWidth,
    // PanelVSyncWidth,
    {"DACOUT_480P_60", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 62, 60, 6, 30, 122, 0, 720, 480, 958, 858, 758, 625, 525, 425, \
        37, 27, 17, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x0073fbde, 0x003555e9, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_576P_50", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 64, 68, 5, 39, 132, 0, 720, 576, 964, 864, 764, 725, 625, 525, \
        37, 27, 17, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x00742b06, 0x00355fe0, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_720P_50", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 40, 220, 5, 20, 260, 0, 1280, 720, 2080, 1980, 1880, 850, 750, 650, \
        84, 74, 64, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x001ebcb1, 0x001770c0, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_720P_60", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 40, 220, 6, 20, 260, 0, 1280, 720, 1750, 1650, 1550, 850, 750, 650, \
        84, 74, 64, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x001ebcb1, 0x001770c0, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080P_50", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 1080, 2740, 2640, 2540, 1225, 1125, 1025, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE,  \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080P_60", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 1080, 2300, 2200, 2100, 1225, 1125, 1025, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE,  \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_480I_60", 0, 0, LINK_DAC_I, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 62, 60, 6, 30, 122, 0, 720, 480, 958, 858, 758, 625, 525, 425, \
        37, 27, 17, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x002cd3fe, 0x001ed4c4, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_576I_50", 0, 0, LINK_DAC_I, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 64, 68, 5, 39, 132, 0, 720, 576, 964, 864, 764, 725, 625, 525, \
        37, 27, 17, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x002ce213, 0x001edb6d, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080I_60", 0, 0, LINK_DAC_I, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 15, 192, 0, 1920, 1080, 2300, 2200, 2100, 1225, 1125, 1025, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE,  \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080I_50", 0, 0, LINK_DAC_I, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 15, 192, 0, 1920, 1080, 2740, 2640, 2540, 1225, 1125, 1025, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE,  \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080P_30", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 1080, 2300, 2200, 2100, 1225, 1125, 1025, \
        84, 74, 64, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080P_25", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 1080, 2300, 2200, 2100, 1225, 1125, 1025, \
        71, 61, 51, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1080P_24", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 1080, 2300, 2200, 2100, 1225, 1125, 1025, \
        69, 59, 49, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_DCLK,0},

    {"DACOUT_640X480P_60", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 96, 48, 2, 33, 144, 0, 640, 480, 900, 800, 700, 625, 525, 425, \
        37, 27, 17, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x0073fbde, 0x003555e9, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_2205P_24", 0, 0, LINK_DAC_P, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 44, 148, 5, 36, 192, 0, 1920, 2205, 2850, 2750, 2650, 2350, 2250, 2150, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 1, 0x001c848e, 0x0018eb59, E_PNL_CHG_DCLK,0},

    {"DACOUT_1470P_50", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 40, 220, 5, 20, 260, 0, 1280, 1470, 2080, 1980, 1880, 1600, 1500, 1400, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x001ebcb1, 0x001770c0, E_PNL_CHG_VTOTAL,0},

    {"DACOUT_1470P_60", 0, 0, LINK_DAC_P, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,   \
        30, 400, 80, 30, 40, 220, 6, 20, 260, 0, 1280, 1470, 1750, 1650, 1550, 1600, 1500, 1400, \
        158, 148, 138, 25, 192, 160, 255, 80, 0, E_PNL_ASPECT_RATIO_WIDE, 0, TI_8BIT_MODE, \
        OUTPUT_10BIT_MODE, 0, 0, 0, 0, 0, 0x001ebcb1, 0x001770c0, E_PNL_CHG_VTOTAL,0},
};

#endif
//**********************************************************************
//** HDMI Analog Type
//**********************************************************************

#define HDMITX_INFO_SIZE     2

static HDMITx_Analog_Param_t astHdmiAnalogCfg[HDMITX_INFO_SIZE] =
{
    // {tm_txcurrent, tm_pren2, tm_precon, tm_pren, tm_tenpre, tm_ten}
    // color depth 24bit, 30bit
    {0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
    // color depth 36bit
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x07},
};
