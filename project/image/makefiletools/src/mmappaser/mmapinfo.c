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
// headers of itself
#include "mmapinfo.h"

// headers of standard C libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// headers of the same layer's
#include "mmapparser.h"


#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)

#define MMAPINFO_DBG(fmt, arg...)                       //printf((char *)fmt, ##arg)
#define MMAPINFO_ERR(fmt, arg...)                       printf((char *)fmt, ##arg)
static MMAP_BOOL m_bCMADisable;
static    MMapInfo_t* SaveInfo(char*, MMAP_BOOL);

static    MMInfo_t m_mminfo;
static    MMAP_U32 m_u32Mmap_count;
static    MMapInfo_t m_pMmapInfos[MMAP_MAX];
static    char m_pMmap_id[MMAP_ID_COUNT][MMAP_MAX_NAME_LEN];

    MMAP_BOOL m_bEnable_miu_1;
    MMAP_BOOL m_bEnable_miu_2;
    MMAP_U32 m_u32Miu0;
    MMAP_U32 m_u32Miu1;
    MMAP_U32 m_u32Miu2;

    MMAP_U32 m_u32Miu0_end_adr;
    MMAP_U32 m_u32Miu1_end_adr;
    MMAP_U32 m_u32Miu2_end_adr;

    MMAP_U32 m_u32CMAValue;
    MMAP_U32 m_u32MiuGroupSelmiuValue[MMAP_MIU_SELECT_NO][MMAP_MIU_SELECT_GROUP][MMAP_MIU_SELECT_BIT];

int ParserMmap(const char *mmap_path, MMAPInfo *info)
{
    //init variable.
    m_bEnable_miu_1=0;
    m_bEnable_miu_2=0;

    m_u32Miu0=0;
    m_u32Miu1=0;
    m_u32Miu2=0;

    m_u32Miu0_end_adr=0;
    m_u32Miu1_end_adr = 0;
    m_u32Miu2_end_adr = 0;

    m_bCMADisable = MMAP_TRUE;
    memset(&m_mminfo,0,sizeof(MMInfo_t));
    MMAP_U16 mmapItemCount = 0;
    MMAPItem* mmapItems;
    mmapItems = (MMAPItem*)malloc(sizeof(MMAPItem) * MAX_ITEM_COUNT);
    ASSERT(mmapItems);
    memset(mmapItems, 0, sizeof(MMAPItem)*MAX_ITEM_COUNT);

    MMAP_U16 mmapidCount = 0;
    MMAPItem mmapids[MMAP_ID_COUNT];

    memset(mmapids, 0, sizeof(MMAPItem)*MMAP_ID_COUNT);
    memset(m_u32MiuGroupSelmiuValue, 0, sizeof(m_u32MiuGroupSelmiuValue));
    MMAPParserHandle handle;
    handle.pu8MMAPParsedItemCount = &mmapItemCount;
    handle.pstMMAPParsedItems = mmapItems;
    handle.pu8MMAPidCount = &mmapidCount;
    handle.pstMMAPids = mmapids;
    if( MMAPFileParser(mmap_path, &handle,NULL) == ERR_NONEXIST_FILE )
    {
        MMAPINFO_ERR("Warning:file %s doesn't exist\n",mmap_path);
        MMAPINFO_ERR("Tip:Only in tee enable, you need to check mmaps.ini open fail.\n");
        return -1;
    }

    m_u32Mmap_count = 0;
    MMAP_U32 i;

    // build MMID-String mapping first
    for(i = 0; i < mmapidCount; i++)
    {
        strcpy(m_pMmap_id[i], mmapids[i].name);
    }

    for(i = 0; i < mmapItemCount; i++)
    {
        if(Is_StrFullCmp("MIU_DRAM_LEN", mmapItems[i].name))
        {
            m_mminfo.total_mem_size = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU_DRAM_LEN0", mmapItems[i].name))
        {
            m_mminfo.miu0_mem_size = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU_DRAM_LEN1", mmapItems[i].name))
        {
            m_mminfo.miu1_mem_size = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU_DRAM_LEN2", mmapItems[i].name))
        {
            m_mminfo.miu2_mem_size = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("ENABLE_MIU_1", mmapItems[i].name))
        {
            m_bEnable_miu_1 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("ENABLE_MIU_2", mmapItems[i].name))
        {
            m_bEnable_miu_2 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU_INTERVAL", mmapItems[i].name))
        {
            m_mminfo.miu_boundary = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU_INTERVAL2", mmapItems[i].name))
        {
            m_mminfo.miu_boundary2 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU0", mmapItems[i].name))
        {
            m_u32Miu0 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU1", mmapItems[i].name))
        {
            m_u32Miu1 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU2", mmapItems[i].name))
        {
            m_u32Miu2 = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU0_END_ADR", mmapItems[i].name))
        {
            m_u32Miu0_end_adr = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU1_END_ADR", mmapItems[i].name))
        {
            m_u32Miu1_end_adr = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MIU2_END_ADR", mmapItems[i].name))
        {
            m_u32Miu2_end_adr = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("MMAP_COUNT", mmapItems[i].name))
        {
            m_u32Mmap_count = mmapItems[i].value;
        }
        else if(Is_StrFullCmp("TEST_4K_ALIGN", mmapItems[i].name))
        {
           m_mminfo.b_is_4k_align = mmapItems[i].value;
        }
        else if(strstr(mmapItems[i].name,MMAP_M_GROUP_SELMIU))
        {
            MMAP_U32 LastLoc=strlen(mmapItems[i].name)-1;
            MMAP_U32 u32GroupNum=mmapItems[i].name[LastLoc]- '0';
            ASSERT(u32GroupNum<MMAP_MIU_SELECT_GROUP);

            char* MiuSubString=NULL;
            char* MiuNo=NULL;
            char* rest_of_str = NULL;
            MiuSubString=strtok_r(mmapItems[i].name,"_",&rest_of_str);
            if(MiuSubString!=NULL)
            {
                char* str = NULL;
                MiuNo=strtok_r(MiuSubString,"MIU",&str);
                ASSERT(strtoul(MiuNo, 0, 0)<MMAP_MIU_SELECT_NO);

                MMAP_U32 u32temp;
                MMAP_U32 u32Count=0;
                MMAPINFO_DBG("MiuNo=%s u32GroupNum=%ld value=%d\n",MiuNo,u32GroupNum,mmapItems[i].value);
                u32temp=mmapItems[i].value;
                do{
                    m_u32MiuGroupSelmiuValue[strtoul(MiuNo, 0, 0)][u32GroupNum][u32Count]=(u32temp % 2);
                    MMAPINFO_DBG("m_u32MiuGroupSelmiuValue[%ld][%ld][%ld]=%ld\n",strtoul(MiuNo, 0, 0),u32GroupNum,u32Count,m_u32MiuGroupSelmiuValue[strtoul(MiuNo, 0, 0)][u32GroupNum][u32Count]);
                    u32temp=(MMAP_S32)u32temp/2;
                    ASSERT(u32Count<MMAP_MIU_SELECT_BIT);
                    u32Count++;
                  }while(u32temp>0);
            }
        }
        else if(Is_StrFullCmp("CMA", mmapItems[i].name))
        {
           m_u32CMAValue = mmapItems[i].value;
        }
        else
        {
            MMapInfo_t *pMmapInfo;
            if ( 0 == memcmp("_LAYER", mmapItems[i].name + (strlen(mmapItems[i].name) - 6), 6))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 6);
                *(tempStr + strlen(mmapItems[i].name) - 6) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u8Layer = mmapItems[i].value;
                }
            }
            else if ( 0 == memcmp("_ALIGN", mmapItems[i].name + (strlen(mmapItems[i].name) - 6), 6))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 6);
                *(tempStr + strlen(mmapItems[i].name) - 6) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u32Align = mmapItems[i].value;
                }
            }
            else if(0 == memcmp("_ADR", mmapItems[i].name + (strlen(mmapItems[i].name) - 4), 4))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 4);
                *(tempStr + strlen(mmapItems[i].name) - 4) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u32Addr = mmapItems[i].value;
                }
            }
            else if(0 == memcmp("_LEN", mmapItems[i].name + (strlen(mmapItems[i].name) - 4), 4))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 4);
                *(tempStr + strlen(mmapItems[i].name) - 4) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u32Size = mmapItems[i].value;
                }
            }
            else if(0 == memcmp("_MEMORY_TYPE", mmapItems[i].name + (strlen(mmapItems[i].name) - 12), 12))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 12);
                *(tempStr + strlen(mmapItems[i].name) - 12) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u32MemoryType = mmapItems[i].value;
                    pMmapInfo->u32MiuNo = ((pMmapInfo->u32MemoryType) & (m_u32Miu1|m_u32Miu2));
                    if(pMmapInfo->u32MiuNo==1)
                    {
                        pMmapInfo->u32Addr=pMmapInfo->u32Addr+m_mminfo.miu_boundary;
                    }
                    else if(pMmapInfo->u32MiuNo==2)
                    {
                        pMmapInfo->u32Addr=pMmapInfo->u32Addr+m_mminfo.miu_boundary2;
                    }
                }
            }
            else if(0 == memcmp("_CMA_HID", mmapItems[i].name + (strlen(mmapItems[i].name) - 8), 8))
            {
                char tempStr[MAX_NAME_LEN];
                strncpy(tempStr, mmapItems[i].name, strlen(mmapItems[i].name) - 8);
                *(tempStr + strlen(mmapItems[i].name) - 8) = 0;
                pMmapInfo = SaveInfo(tempStr, MMAP_TRUE);
                if(pMmapInfo)
                {
                    pMmapInfo->u32CMAHid = mmapItems[i].value;
                }
                //[for MI] check all HID value
                m_bCMADisable &= (mmapItems[i].value == 0);
            }
        }
    }

    free(mmapItems);
#if (SYSINFO_DEBUG == 1)
    printf("-----------------MMAPCfg Start---------------------\n");
    printf("total_mem_size = %u MB, miu0_mem_size = %u MB, miu1_mem_size = %u MB, miu2_mem_size = %u MB, \
        miu_boundary = %u MB, miu_boundary2 = %u MB, n_mmap_items = %u , b_is_4k_align = %d\n", m_mminfo.total_mem_size / (1024 * 1024), \
           m_mminfo.miu0_mem_size / (1024 * 1024), m_mminfo.miu1_mem_size / (1024 * 1024), m_mminfo.miu2_mem_size / (1024 * 1024), \
           m_mminfo.miu_boundary / (1024 * 1024),m_mminfo.miu_boundary2 / (1024 * 1024), m_mminfo.n_mmap_items, m_mminfo.b_is_4k_align);
    for(int i = 0 ; i < (int)m_mminfo.n_mmap_items ; i++)
    {
        printf("MMAP[%02u] = 0x%08x len:0x%08x  layer = %u align = 0x%08x (MIU%d)\n", \
               m_pMmapInfos[i].u32gID, m_pMmapInfos[i].u32Addr, m_pMmapInfos[i].u32Size, m_pMmapInfos[i].u8Layer, \
               m_pMmapInfos[i].u32Align, m_pMmapInfos[i].u32MiuNo);
    }
    printf("-----------------MMAPCfg End-----------------------\n\n");
#endif
    info->header = &m_mminfo;
    info->name = m_pMmap_id;
    info->context = m_pMmapInfos;
    return 0;
}

MMAP_U16 StrToMMAPID(const char* pName)
{
    MMAP_U16 i;
    for(i = 0; i < MMAP_ID_COUNT; i++)
    {
        if(Is_StrFullCmp(m_pMmap_id[i], pName))
        {
            return i;
        }
    }
    return MMAP_ID_INVALID_ID;
}


MMapInfo_t* SaveInfo(char* name, MMAP_BOOL create)
{
    unsigned int i;
    MMAP_U16 u16_id = StrToMMAPID(name);

    if(u16_id == MMAP_ID_INVALID_ID)
    {
        return NULL;
    }
    for(i = 0; i < m_mminfo.n_mmap_items; i++)
    {
        if(m_pMmapInfos[i].u32gID == u16_id)
        {
            return &m_pMmapInfos[i];
        }
    }

    if(create)
    {
        if(i < MMAP_MAX)
        {
            m_pMmapInfos[i].u32gID = u16_id;
            m_pMmapInfos[i].u32Addr = 0;
            m_pMmapInfos[i].u32Size = 0;
            m_mminfo.n_mmap_items++;
            return &m_pMmapInfos[i];
        }
        else
        {
            MMAPINFO_ERR("Error: the MMAP array is filled...\n");
            return NULL;
        }
    }

    MMAPINFO_ERR("Error: not finded MMAP_ID:[%u] in MMAP array...\n", u16_id);
    return NULL;
}
