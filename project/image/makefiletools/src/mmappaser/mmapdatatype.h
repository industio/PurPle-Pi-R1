//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _MMAP_INFO_DATATYPE_H_
#define _MMAP_INFO_DATATYPE_H_

typedef enum
{
    E_MMAP_Original = 0,
    E_MMAP_Security,
    E_MMAP_MAX
} EN_MMAP_Type;

typedef enum
{
  ERR_NONEXIST_FILE = -1,
  ERR_POINTER_OVERFLOW= -2,
  ERR_NULL_POINTER= -3,
}PARSER_Err;

typedef enum
{
  ERR_MMAP_ID = -1,
  ERR_MIU_OVERBOUND= -2,
}MIUNO_Err;

typedef enum
{
    E_MMAPCheck_CommonRule = 0,
    E_MMAPCheck_ProjectRule,
    E_MMAPCheck_MAX
} EN_MMAPCheck_Rule;


#define MMAP_PATH                      "/config/mmap.ini"
#define MMAP_PATH_S                    "/config/mmaps.ini"
#define MMAP_PATH_SECURE               "/config/mmap.aes"     //temp mmap file
#define CONFIG_PATH                    "/config/mmap"         //auto fit mmap path
#define MMAP_M_NEED                    "M_NEED"
#define MMAP_M_CANNOT_COBUFFER         "M_CANNOT_COBUFFER"
#define MMAP_M_SAME_MIU                "M_SAME_MIU"
#define MMAP_M_ORDER_ADJACENT          "M_ORDER_ADJACENT"
#define MMAP_M_ORDER                   "M_ORDER"
#define MMAP_M_LIMIT_SIZE              "M_LIMIT_SIZE"
#define MMAP_M_LIMIT_LOCATE            "M_LIMIT_LOCATE"
#define MMAP_M_GROUP_SELMIU            "GROUP_SELMIU"
#define MMAP_M_NEED_BY_COMPILE_FLAG    "M_NEED_BY_COMPILE_FLAG"
#define MMAP_TRUE 1
#define MMAP_FALSE 0
typedef unsigned char       MMAP_BOOL;
typedef unsigned char       MMAP_U8;    // 1 byte
typedef unsigned short      MMAP_U16;   // 2 bytes
typedef unsigned int        MMAP_U32;   // 4 bytes
typedef unsigned long long  MMAP_U64;   // 8 bytes

typedef signed char         MMAP_S8;    // 1 byte
typedef signed short        MMAP_S16;   // 2 bytes
typedef signed int          MMAP_S32;   // 4 bytes
typedef signed long long    MMAP_S64;   // 8 bytes


/**********************************************************************************/
/* Warning , MMAP static enumerate table only can be added from the end of last element.
 *  Please contact Ted.Chen if you wanna to modify this table
 */

#define MAX_ITEM_COUNT 810
#define MAX_NAME_LEN   100

#define MMAP_MAX      133
#define MMAP_ID_INVALID_ID      0xFFFF
#define MMAP_ID_INVALID_ADDRESS 0xFFFFFFFF

#define MMAP_ID_COUNT 100
#define MMAP_MAX_NAME_LEN 1024
#define CMDLINE_PATH "/proc/cmdline"
#define MMAP_MIU_SELECT_NO 3
#define MMAP_MIU_SELECT_GROUP 7
#define MMAP_MIU_SELECT_BIT 16

typedef struct MMInfo_s
{
    MMAP_U32    total_mem_size;
    MMAP_U32    miu0_mem_size;
    MMAP_U32    miu1_mem_size;
    MMAP_U32    miu2_mem_size;
    MMAP_U32    miu_boundary;
    MMAP_U32    miu_boundary2;
    MMAP_U32    n_mmap_items;
    MMAP_BOOL        b_is_4k_align;
} MMInfo_t;

typedef struct MMapInfo_s
{
    MMAP_U32    u32gID;
    MMAP_U32    u32Addr;
    MMAP_U32    u32Size;
    MMAP_U8     u8Layer;
    MMAP_U32    u32Align;
    MMAP_U32    u32MemoryType;
    MMAP_U32    u32MiuNo;
    MMAP_U32    u32CMAHid; 
} MMapInfo_t;

typedef struct
{
    MMInfo_t m_MMinfo;
    MMapInfo_t *m_MMtable;
} MMAPCfgBlock_t;

typedef struct
{
    char name[MAX_NAME_LEN];
    unsigned int value;
} MMAPItem;


typedef struct
{
    unsigned int key;
    char name[MAX_NAME_LEN];
} MMAPCheckItem;


typedef struct
{
    unsigned short* pu8MMAPParsedItemCount;
    MMAPItem* pstMMAPParsedItems;
    unsigned short* pu8MMAPidCount;
    MMAPItem* pstMMAPids;
} MMAPParserHandle;


#endif // _MMAP_INFO_DATATYPE_H_

