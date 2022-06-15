/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _PQ_H_
#define _PQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PQ_PICTURE_EN           0
#define PQ_GAMMA_EN             0
#define PQ_PEAKING_EN           0

#define PQ_COLOR_ID             1

#define PQ_PICUTRE_HUE          0x32
#define PQ_PICTURE_SATURATION   0x80
#define PQ_PICTURE_R_GAIN       0x80
#define PQ_PICTURE_G_GAIN       0x80
#define PQ_PICTURE_B_GAIN       0x80
#define PQ_PICTURE_CONTRAST     0x80
#define PQ_PICTURE_R_BRIGHTNESS 0x80
#define PQ_PICTURE_G_BRIGHTNESS 0x80
#define PQ_PICTURE_B_BRIGHTNESS 0x80


HalDispPqPeakingConfig_t stPqPeaking =
{
    PQ_PEAKING_EN, //bUpdate
    0x00, //u8LtiEn;
    0x00, //u8LtiCoef;
    0x00, //u8LtiStep;
    0x00, //u8LtiMediaFilterOn
    0x00, //u8CtiEn;
    0x00, //u8CtiCoef;
    0x00, //u8CtiStep;
    0x00, //u8CtiMediaFilterOn
    0x00, //u8DiffAdapEn;
    0x00, //u8PosLimitTh;
    0x00, //u8NegLimitTh;
    0x00, //u8PeakingEn;
    0x00, //u8Band1Coef;
    0x00, //u8Band1Step;
    0x00, //u8Band1DiffAdapEn;
    0x00, //u8Band1PosLimitTh;
    0x00, //u8Band1NegLimitTh;
    0x00, //u8Band2Coef;
    0x00, //u8Band2Step;
    0x00, //u8Band2DiffAdapEn;
    0x00, //u8Band2PosLimitTh;
    0x00, //u8Band2NegLimitTh;
    0x00, //u8CoringTh1;
    0x00, //u8CoringTh2;
    0x00, //u8PeakingTerm1Sel;
    0x00, //u8PeakingTerm2Sel;
    0x00, //u8PeakingTerm3Sel;
    0x00, //u8PeakingTerm4Sel;
};

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
HalDispPqGammaConfig_t stPqGamma =
{
    PQ_GAMMA_EN, //bUpdate
    0x01, //u8En
   {0x00, // Gamma_R
    0x08,
    0x10,
    0x18,
    0x20,
    0x28,
    0x30,
    0x38,
    0x40,
    0x48,
    0x50,
    0x58,
    0x60,
    0x68,
    0x70,
    0x78,
    0x80,
    0x88,
    0x90,
    0x98,
    0xA0,
    0xA8,
    0xB0,
    0xB8,
    0xC0,
    0xC8,
    0xD0,
    0xD8,
    0xE0,
    0xE8,
    0xF0,
    0xF8,
    0xFF},

   {0x00, //Gamm_G
    0x08,
    0x10,
    0x18,
    0x20,
    0x28,
    0x30,
    0x38,
    0x40,
    0x48,
    0x50,
    0x58,
    0x60,
    0x68,
    0x70,
    0x78,
    0x80,
    0x88,
    0x90,
    0x98,
    0xA0,
    0xA8,
    0xB0,
    0xB8,
    0xC0,
    0xC8,
    0xD0,
    0xD8,
    0xE0,
    0xE8,
    0xF0,
    0xF8,
    0xFF},

   {0x00, //GammaB
    0x08,
    0x10,
    0x18,
    0x20,
    0x28,
    0x30,
    0x38,
    0x40,
    0x48,
    0x50,
    0x58,
    0x60,
    0x68,
    0x70,
    0x78,
    0x80,
    0x88,
    0x90,
    0x98,
    0xA0,
    0xA8,
    0xB0,
    0xB8,
    0xC0,
    0xC8,
    0xD0,
    0xD8,
    0xE0,
    0xE8,
    0xF0,
    0xF8,
    0xFF},
};


HalDispInternalConfig_t stPqColorId =
{
    E_HAL_DISP_INTER_CFG_COLORID,
    0,
    PQ_COLOR_ID,
};


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------



#endif
