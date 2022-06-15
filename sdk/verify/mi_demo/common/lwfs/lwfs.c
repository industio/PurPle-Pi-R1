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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include "list.h"
#include "lwfs.h"


#define PRINT_ERR(fmt, args...)      ({do{fprintf(stderr, fmt, ##args);}while(0);})
#if 0
#define PRINT(fmt, args...)      ({do{printf(fmt, ##args);}while(0);})
#else
#define PRINT(fmt, args...)
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (( (val)/(alignment))*(alignment))
#endif

#ifndef ASSERT
#define ASSERT(x)                       \
do {                                    \
    if (!(x)) {                         \
        PRINT("assertion failed %s: %d: %s\n", \
               __FILE__, __LINE__, #x);             \
        assert(0);                          \
    }                               \
} while (0)
#endif

typedef struct
{
    struct list_head stRamDataList;
    void *pDramData;
    u32 u32RamSize;
}SS_LWFS_FileDataDesc_t;

typedef struct
{
    struct list_head stList;
    struct list_head stRamDataList;
    SS_LWFS_FileHeaderData_t stFileHeader;
    SS_LWFS_FileOpertion_e enFileOpt;
    u32 u32BlockSize;
    u32 u32FilePos;
    s32 s32BlockFd;
    u32 u32FileAlign;
    u32 u32TotalRamSz;
}SS_LWFS_FileDesc_t;

static s32 _FileOpen(const s8 *s8Path, s32 s32Mode)
{
    return open(s8Path, s32Mode);
}
static s32 _FileClose(s32 s32Fd)
{
    return close(s32Fd);
}
static s32 _FileSeek(s32 s32Fd, s32 s32Offset, s32 s32Whence)
{
    return lseek(s32Fd, s32Offset, s32Whence);
}
static s32 _FileRead(s32 s32Fd, void *pBuf, s32 s32Size)
{
    s32 s32ReadCnt = 0;
    s32 s32Ret = 0;
    do
    {
        s32Ret = read(s32Fd, pBuf, s32Size - s32ReadCnt);
        if (s32Ret < 0)
        {
            perror("read");
            return -1;
        }
        if (s32Ret == 0)
        {
            break;
        }
        s32ReadCnt += s32Ret;
        pBuf += s32Ret;
    }while(s32ReadCnt < s32Size);

    return s32ReadCnt;
}
static s32 _FileWrite(s32 s32Fd, void *pBuf, s32 s32Size)
{
    s32 s32WriteCnt = 0;
    s32 s32Ret = 0;
    do
    {
        s32Ret = write(s32Fd, pBuf, s32Size - s32WriteCnt);
        if (s32Ret < 0)
        {
            perror("write");
            return -1;
        }
        if (s32Ret == 0)
        {
            break;
        }
        s32WriteCnt += s32Ret;
        pBuf += s32Ret;
    }while(s32WriteCnt < s32Size);

    return s32WriteCnt;
}
s32 SS_LWFS_List(const s8 *s8MtdPath)
{
    s32 s32Fd = 0;
    s32 s32Ret = 0;

    u32 i = 0;
    SS_LWFS_PartitionHeaderData_t stBlockHeader;
    SS_LWFS_FileHeaderData_u uFileHeadeData;

    s32Fd = _FileOpen(s8MtdPath, O_RDONLY);
    if (s32Fd < 0)
    {
        perror("open");

        return -1;
    }

    if (_FileRead(s32Fd, &stBlockHeader, sizeof(SS_LWFS_PartitionHeaderData_t)) < 0)
    {
        s32Ret = -1;

        goto ERR;
    }
    if (stBlockHeader.u32MagicId != LWFS_MAGIC_ID)
    {
        PRINT_ERR("MAGIC ID NOT MATCH! EXPECT : %d, CURRENT : %d\n", LWFS_MAGIC_ID, stBlockHeader.u32MagicId);

        goto ERR;
    }
    _FileSeek(s32Fd, sizeof(SS_LWFS_PartitionHeaderData_u), SEEK_SET);
    printf("File count %d\n", stBlockHeader.u32FileCount);
    printf("File size:    Padding:    Offset:    File name:\n");
    for (i = 0; i < stBlockHeader.u32FileCount; i++)
    {
        if (_FileRead(s32Fd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
        {
            s32Ret = -1;

            goto ERR;
        }
        printf("%10d    %8d    %7d    %10s\n", uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32FilePosition, uFileHeadeData.stData.s8FileName);
    }
ERR:
    _FileClose(s32Fd);

    return s32Ret;
}
s32 SS_LWFS_Open(const s8 *s8MtdPath, const s8 *s8FileName, SS_LWFS_FileOpertion_e enFileOpt)
{
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;
    SS_LWFS_FileDesc_t *pstFileDescN = NULL;
    SS_LWFS_FileDesc_t *pstFileDescM = NULL;
    SS_LWFS_PartitionHeaderData_t stBlockHeader;
    SS_LWFS_FileHeaderData_u uFileHeadeData;
    SS_LWFS_FileDataDesc_t *pstRamData = NULL;
    SS_LWFS_FileDataDesc_t *pos = NULL, *posN = NULL;
    struct list_head stHead;
    u32 u32CrcCheck = 0;
    u32 i = 0;
    u32 u32CurPos = 0;

    pstFileDesc = (SS_LWFS_FileDesc_t *)malloc(sizeof(SS_LWFS_FileDesc_t));
    ASSERT(pstFileDesc);
    memset(pstFileDesc, 0, sizeof(SS_LWFS_FileDesc_t));
    INIT_LIST_HEAD(&pstFileDesc->stList);
    INIT_LIST_HEAD(&pstFileDesc->stRamDataList);

    pstFileDesc->enFileOpt = enFileOpt;
    switch (pstFileDesc->enFileOpt)
    {
        case EN_LWFS_FILE_READ_ONLY:
        {
            pstFileDesc->s32BlockFd = _FileOpen(s8MtdPath, O_RDONLY);
        }
        break;
        case EN_LWFS_FILE_READ_WRITE_TRUNC:
        case EN_LWFS_FILE_READ_WRITE:
        {
            pstFileDesc->s32BlockFd = _FileOpen(s8MtdPath, O_RDWR);
        }
        break;
        default:
            PRINT_ERR("File opt error: %d\n", enFileOpt);
            return -1;
    }
    if (pstFileDesc->s32BlockFd < 0)
    {
        perror("SS_LWFS_Open");
        free(pstFileDesc);

        return -1;
    }

    pstFileDesc->u32BlockSize = (u32)_FileSeek(pstFileDesc->s32BlockFd, 0, SEEK_END);
    _FileSeek(pstFileDesc->s32BlockFd, 0, SEEK_SET);
    if (_FileRead(pstFileDesc->s32BlockFd, &stBlockHeader, sizeof(SS_LWFS_PartitionHeaderData_t)) < 0)
    {
        goto ERR;
    }
    if (stBlockHeader.u32MagicId != LWFS_MAGIC_ID)
    {
        PRINT_ERR("MAGIC ID NOT MATCH! EXPECT : %d, CURRENT : %d\n", LWFS_MAGIC_ID, stBlockHeader.u32MagicId);

        goto ERR;
    }
    _FileSeek(pstFileDesc->s32BlockFd, sizeof(SS_LWFS_PartitionHeaderData_u), SEEK_SET);
    for (i = 0; i < stBlockHeader.u32FileCount; i++)
    {
        if (_FileRead(pstFileDesc->s32BlockFd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
        {
            goto ERR;
        }
        PRINT("File name %s, pos 0x%x file size 0x%x padding 0x%x crc 0x%x\n", uFileHeadeData.stData.s8FileName, uFileHeadeData.stData.u32FilePosition, uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32CrcCheck);
        if (pstFileDescN)
        {
            pstFileDescM = (SS_LWFS_FileDesc_t *)malloc(sizeof(SS_LWFS_FileDesc_t));
            ASSERT(pstFileDescM);
            PRINT("Add: %p\n", pstFileDescM);
            memset(pstFileDescM, 0, sizeof(SS_LWFS_FileDesc_t));
            pstFileDescM->enFileOpt = EN_LWFS_FILE_READ_ONLY;
            pstFileDescM->s32BlockFd = pstFileDescN->s32BlockFd;
            pstFileDescM->u32BlockSize = pstFileDescN->u32BlockSize;
            pstFileDescM->u32FileAlign = pstFileDescN->u32FileAlign;
            INIT_LIST_HEAD(&pstFileDescM->stRamDataList);
            memcpy(&pstFileDescM->stFileHeader, &uFileHeadeData.stData, sizeof(SS_LWFS_FileHeaderData_t));
            list_add(&pstFileDescM->stList, &pstFileDescN->stList);
            pstRamData = (SS_LWFS_FileDataDesc_t *)malloc(sizeof(SS_LWFS_FileDataDesc_t));
            ASSERT(pstRamData);
            memset(pstRamData, 0, sizeof(SS_LWFS_FileDataDesc_t));
            pstRamData->u32RamSize = pstFileDescM->stFileHeader.u32FileLen + pstFileDescM->stFileHeader.u32FilePaddingLen;
            pstRamData->pDramData = malloc(pstRamData->u32RamSize);
            ASSERT(pstRamData->pDramData);
            list_add_tail(&pstRamData->stRamDataList, &pstFileDescM->stRamDataList);
            pstFileDescM->u32TotalRamSz = pstRamData->u32RamSize;
            u32CurPos = (u32)_FileSeek(pstFileDescM->s32BlockFd, 0, SEEK_CUR);
            _FileSeek(pstFileDescM->s32BlockFd, pstFileDescM->stFileHeader.u32FilePosition, SEEK_SET);
            if (_FileRead(pstFileDescM->s32BlockFd, pstRamData->pDramData, pstFileDescM->stFileHeader.u32FileLen) < 0)
            {
                goto ERR;
            }
            u32CrcCheck = (u32)crc32(0, pstRamData->pDramData, pstFileDescM->stFileHeader.u32FileLen);
            if (u32CrcCheck != uFileHeadeData.stData.u32CrcCheck)
            {
                PRINT_ERR("File %s checkcrc error! cal 0x%x bin 0x%x\n", uFileHeadeData.stData.s8FileName, u32CrcCheck, uFileHeadeData.stData.u32CrcCheck);
                goto ERR;
            }
            PRINT("Add: dram node %p dram_addr %p, file size 0x%x\n",pstRamData, pstRamData->pDramData, pstFileDescM->stFileHeader.u32FileLen);
            _FileSeek(pstFileDescM->s32BlockFd, (s32)u32CurPos, SEEK_SET);
            pstFileDescN = pstFileDescM;
            pstFileDescM = NULL;
        }
        else if (!strcmp(uFileHeadeData.stData.s8FileName, s8FileName))
        {
            PRINT("Create: %p\n", pstFileDesc);
            memcpy(&pstFileDesc->stFileHeader, &uFileHeadeData.stData, sizeof(SS_LWFS_FileHeaderData_t));
            pstFileDesc->u32FileAlign = stBlockHeader.u32FileAlignement;
            pstRamData = (SS_LWFS_FileDataDesc_t *)malloc(sizeof(SS_LWFS_FileDataDesc_t));
            ASSERT(pstRamData);
            memset(pstRamData, 0, sizeof(SS_LWFS_FileDataDesc_t));
            if (EN_LWFS_FILE_READ_WRITE_TRUNC == pstFileDesc->enFileOpt)
            {
                pstRamData->u32RamSize = pstFileDesc->u32FileAlign;
                pstRamData->pDramData = malloc(pstRamData->u32RamSize);
                ASSERT(pstRamData->pDramData);
                memset(pstRamData->pDramData, 0, pstRamData->u32RamSize);
                list_add_tail(&pstRamData->stRamDataList, &pstFileDesc->stRamDataList);
                pstFileDesc->u32TotalRamSz = pstRamData->u32RamSize;
                pstFileDesc->stFileHeader.u32FileLen = 0;
                pstFileDesc->stFileHeader.u32FilePaddingLen = pstRamData->u32RamSize;
            }
            else
            {
                pstRamData->u32RamSize = pstFileDesc->stFileHeader.u32FileLen + pstFileDesc->stFileHeader.u32FilePaddingLen;
                pstRamData->pDramData = malloc(pstRamData->u32RamSize);
                ASSERT(pstRamData->pDramData);
                list_add_tail(&pstRamData->stRamDataList, &pstFileDesc->stRamDataList);
                pstFileDesc->u32TotalRamSz = pstRamData->u32RamSize;

                u32CurPos = (u32)_FileSeek(pstFileDesc->s32BlockFd, 0, SEEK_CUR);
                _FileSeek(pstFileDesc->s32BlockFd, pstFileDesc->stFileHeader.u32FilePosition, SEEK_SET);
                if (_FileRead(pstFileDesc->s32BlockFd, pstRamData->pDramData, pstFileDesc->stFileHeader.u32FileLen) < 0)
                {
                    goto ERR;
                }
                u32CrcCheck = (u32)crc32(0, pstRamData->pDramData, pstFileDesc->stFileHeader.u32FileLen);
                if (u32CrcCheck != uFileHeadeData.stData.u32CrcCheck)
                {
                    PRINT_ERR("File %s checkcrc error! cal 0x%x bin 0x%x\n", uFileHeadeData.stData.s8FileName, u32CrcCheck, uFileHeadeData.stData.u32CrcCheck);
                    goto ERR;
                }
                _FileSeek(pstFileDesc->s32BlockFd, (s32)u32CurPos, SEEK_SET);
            }
            PRINT("Create: dram node %p dram_addr %p, file size 0x%x\n",pstRamData, pstRamData->pDramData, pstFileDesc->stFileHeader.u32FileLen);
            pstFileDescN = pstFileDesc;
        }
    }
    if (!pstFileDescN)
    {
        //File didn't exist.
        PRINT_ERR("File didn't exist!\n");
        goto ERR;
    }

    return (u32)pstFileDesc;

ERR:

    _FileClose(pstFileDesc->s32BlockFd);
    list_add_tail(&stHead, &pstFileDesc->stList);
    list_for_each_entry_safe(pstFileDescN, pstFileDescM, &stHead, stList)
    {
        list_for_each_entry_safe(pos, posN, &pstFileDescN->stRamDataList, stRamDataList)
        {
            if (pos->pDramData)
            {
                free(pos->pDramData);
            }
            list_del(&pos->stRamDataList);
            free(pos);
        }
        list_del(&pstFileDescN->stList);
        free(pstFileDescN);
    }

    return -1;
}
s32 SS_LWFS_Read(s32 s32Fd, void *buf, s32 size)
{
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;
    SS_LWFS_FileDataDesc_t *pos = NULL;
    u32 u32TempPos = 0;
    u32 u32GapSize = 0;
    u32 u32BufReadCnt = 0;

    pstFileDesc = (SS_LWFS_FileDesc_t *)s32Fd;
    if (!pstFileDesc)
        return -1;
    if (!size)
        return 0;

    list_for_each_entry(pos, &pstFileDesc->stRamDataList, stRamDataList)
    {
        if (pstFileDesc->u32FilePos - u32TempPos < pos->u32RamSize)
        {
            u32GapSize = min((u32)size - u32BufReadCnt, pos->u32RamSize - (pstFileDesc->u32FilePos - u32TempPos));
            u32GapSize = min(u32GapSize, pstFileDesc->stFileHeader.u32FileLen - pstFileDesc->u32FilePos);
            memcpy(buf + u32BufReadCnt, pos->pDramData + (pstFileDesc->u32FilePos - u32TempPos), u32GapSize);
            u32BufReadCnt += u32GapSize;
            pstFileDesc->u32FilePos += u32GapSize;
        }
        u32TempPos += pos->u32RamSize;
    }

    return u32BufReadCnt;
}

s32 SS_LWFS_Write(s32 s32Fd, void *buf, s32 size)
{
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;
    SS_LWFS_FileDataDesc_t *pos = NULL;
    u32 u32TempPos = 0;
    u32 u32GapSize = 0;
    u32 u32BufWriteCnt = 0;

    pstFileDesc = (SS_LWFS_FileDesc_t *)s32Fd;
    if (!pstFileDesc)
        return -1;
    if (!size)
        return 0;

    /*1.Adjust the buffer alocation.*/
    if (pstFileDesc->u32FilePos + size > pstFileDesc->u32TotalRamSz)
    {
        u32 u32BufAllocCnt = 0;
        u32 u32ExtBufSz = 0;
        u32 u32Idx = 0;
        SS_LWFS_FileDataDesc_t *pstRamData = NULL;

        u32ExtBufSz = pstFileDesc->u32FilePos + size - pstFileDesc->u32TotalRamSz;
        u32BufAllocCnt = u32ExtBufSz / pstFileDesc->u32FileAlign + 1;
        for (u32Idx = 0; u32Idx < u32BufAllocCnt; u32Idx++)
        {
            pstRamData = (SS_LWFS_FileDataDesc_t *)malloc(sizeof(SS_LWFS_FileDataDesc_t));
            ASSERT(pstRamData);
            memset(pstRamData, 0, sizeof(SS_LWFS_FileDataDesc_t));
            pstRamData->pDramData = malloc(pstFileDesc->u32FileAlign);
            ASSERT(pstRamData);
            memset(pstRamData->pDramData, 0, pstFileDesc->u32FileAlign);
            pstRamData->u32RamSize = pstFileDesc->u32FileAlign;
            pstFileDesc->u32TotalRamSz += pstFileDesc->u32FileAlign;
            list_add_tail(&pstRamData->stRamDataList, &pstFileDesc->stRamDataList);
        }
    }
    /*2.Find the pointer var pos val and do memory copy.*/
    list_for_each_entry(pos, &pstFileDesc->stRamDataList, stRamDataList)
    {
        if (pstFileDesc->u32FilePos - u32TempPos < pos->u32RamSize)
        {
            u32GapSize = min((u32)size - u32BufWriteCnt, pos->u32RamSize - (pstFileDesc->u32FilePos - u32TempPos));
            memcpy(pos->pDramData + (pstFileDesc->u32FilePos - u32TempPos), buf + u32BufWriteCnt, u32GapSize);
            u32BufWriteCnt += u32GapSize;
            pstFileDesc->u32FilePos += u32GapSize;
            if (pstFileDesc->u32FilePos > pstFileDesc->stFileHeader.u32FileLen)
            {
                pstFileDesc->stFileHeader.u32FileLen = pstFileDesc->u32FilePos;
                pstFileDesc->stFileHeader.u32FilePaddingLen = pstFileDesc->u32TotalRamSz - pstFileDesc->u32FilePos;
            }
        }
        u32TempPos += pos->u32RamSize;
    }

    return u32BufWriteCnt;
}
s32 SS_LWFS_Sync(s32 s32Fd)
{
    SS_LWFS_FileHeaderData_u uFileHeadeData;
    SS_LWFS_PartitionHeaderData_t stBlockHeader;
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;
    SS_LWFS_FileDataDesc_t *pos = NULL;
    SS_LWFS_FileDesc_t *pstFileDescN = NULL;
    u32 u32CurPos = 0;
    u32 u32TempPos = 0;
    u32 u32CrcCalLen = 0;
    u32 u32FilePos = 0;
    u32 i = 0;

    pstFileDesc = (SS_LWFS_FileDesc_t *)s32Fd;
    if (!pstFileDesc)
        return -1;

    _FileSeek(pstFileDesc->s32BlockFd, 0, SEEK_SET);
    if (_FileRead(pstFileDesc->s32BlockFd, &stBlockHeader, sizeof(SS_LWFS_PartitionHeaderData_t)) < 0)
    {
        return -1;
    }
    _FileSeek(pstFileDesc->s32BlockFd, sizeof(SS_LWFS_PartitionHeaderData_u), SEEK_SET);
    for (i = 0; i < stBlockHeader.u32FileCount; i++)
    {
        if (_FileRead(pstFileDesc->s32BlockFd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
        {
            return -1;
        }
        if (!strcmp(uFileHeadeData.stData.s8FileName, pstFileDesc->stFileHeader.s8FileName))
        {
            u32CurPos = _FileSeek(pstFileDesc->s32BlockFd, 0, SEEK_CUR) - sizeof(SS_LWFS_FileHeaderData_u);
            u32FilePos = pstFileDesc->stFileHeader.u32FilePosition;

            break;
        }
        PRINT("File name %s, file size 0x%x padding 0x%x crc 0x%x\n", uFileHeadeData.stData.s8FileName, uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32CrcCheck);
    }
    if (u32CurPos)
    {
        u32TempPos = 0;
        uFileHeadeData.stData.u32CrcCheck = 0;
        /*Write file*/
        _FileSeek(pstFileDesc->s32BlockFd, u32FilePos, SEEK_SET);
        list_for_each_entry(pos, &pstFileDesc->stRamDataList, stRamDataList)
        {
            if (u32TempPos + pos->u32RamSize > pstFileDesc->stFileHeader.u32FileLen)
            {
                u32CrcCalLen = pstFileDesc->stFileHeader.u32FileLen - u32TempPos;
            }
            else
            {
                u32CrcCalLen = pos->u32RamSize;
            }
            uFileHeadeData.stData.u32CrcCheck = (u32)crc32(uFileHeadeData.stData.u32CrcCheck, pos->pDramData, u32CrcCalLen);
            if (_FileWrite(pstFileDesc->s32BlockFd, pos->pDramData, pos->u32RamSize) == -1)
            {
                return -1;
            }
            u32TempPos += pos->u32RamSize;
        }
        /*Write header*/
        _FileSeek(pstFileDesc->s32BlockFd, u32CurPos, SEEK_SET);
        uFileHeadeData.stData.u32FileLen = pstFileDesc->stFileHeader.u32FileLen;
        uFileHeadeData.stData.u32FilePaddingLen = pstFileDesc->stFileHeader.u32FilePaddingLen;
        uFileHeadeData.stData.u32FilePosition = u32FilePos;
        if (_FileWrite(pstFileDesc->s32BlockFd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
        {
            return -1;
        }
        PRINT("File name %s, pos 0x%x, file size 0x%x padding 0x%x crc 0x%x\n", uFileHeadeData.stData.s8FileName, uFileHeadeData.stData.u32FilePosition, uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32CrcCheck);
        u32FilePos += pstFileDesc->stFileHeader.u32FileLen + pstFileDesc->stFileHeader.u32FilePaddingLen;
        list_for_each_entry(pstFileDescN, &pstFileDesc->stList, stList)
        {
            /*Read head.*/
            u32CurPos = _FileSeek(pstFileDescN->s32BlockFd, 0, SEEK_CUR);
            if (_FileRead(pstFileDescN->s32BlockFd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
            {
                return -1;
            }
            if (u32FilePos == pstFileDescN->stFileHeader.u32FilePosition)
            {
                PRINT("Rest files do not need rebuid!\n");

                break;
            }
            PRINT("Before File name %s, pos 0x%x file size 0x%x padding 0x%x crc 0x%x\n", uFileHeadeData.stData.s8FileName, uFileHeadeData.stData.u32FilePosition, uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32CrcCheck);

            /*Write file.*/
            _FileSeek(pstFileDescN->s32BlockFd, u32FilePos, SEEK_SET);
            u32TempPos = 0;
            uFileHeadeData.stData.u32CrcCheck = 0;
            list_for_each_entry(pos, &pstFileDescN->stRamDataList, stRamDataList)
            {
                if (u32TempPos + pos->u32RamSize > pstFileDescN->stFileHeader.u32FileLen)
                {
                    u32CrcCalLen = pstFileDescN->stFileHeader.u32FileLen - u32TempPos;
                }
                else
                {
                    u32CrcCalLen = pos->u32RamSize;
                }
                uFileHeadeData.stData.u32CrcCheck = (u32)crc32(uFileHeadeData.stData.u32CrcCheck, pos->pDramData, u32CrcCalLen);
                if (_FileWrite(pstFileDescN->s32BlockFd, pos->pDramData, pos->u32RamSize) == -1)
                {
                    return -1;
                }
                u32TempPos += pos->u32RamSize;
            }

            /*Write head.*/
            _FileSeek(pstFileDescN->s32BlockFd, u32CurPos, SEEK_SET);
            uFileHeadeData.stData.u32FileLen = pstFileDescN->stFileHeader.u32FileLen;
            uFileHeadeData.stData.u32FilePaddingLen = pstFileDescN->stFileHeader.u32FilePaddingLen;
            uFileHeadeData.stData.u32FilePosition = u32FilePos;
            if (_FileWrite(pstFileDescN->s32BlockFd, &uFileHeadeData, sizeof(SS_LWFS_FileHeaderData_u)) < 0)
            {
                return -1;
            }
            u32FilePos += pstFileDescN->stFileHeader.u32FileLen + pstFileDescN->stFileHeader.u32FilePaddingLen;
            PRINT("File name %s, pos 0x%x file size 0x%x padding 0x%x crc 0x%x\n", uFileHeadeData.stData.s8FileName, uFileHeadeData.stData.u32FilePosition, uFileHeadeData.stData.u32FileLen, uFileHeadeData.stData.u32FilePaddingLen, uFileHeadeData.stData.u32CrcCheck);

        }
    }

    return 0;
}
s32 SS_LWFS_Seek(s32 s32Fd, s32 s32Offset, s32 s32Whence)
{
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;

    pstFileDesc = (SS_LWFS_FileDesc_t *)s32Fd;
    if (!pstFileDesc)
        return -1;

    switch (s32Whence)
    {
        case EN_LWFS_SEEK_SET:
        {
            pstFileDesc->u32FilePos = s32Offset;
        }
        break;
        case EN_LWFS_SEEK_CUR:
        {
            pstFileDesc->u32FilePos = (s32)pstFileDesc->u32FilePos + s32Offset;
        }
        break;
        case EN_LWFS_SEEK_END:
        {
            pstFileDesc->u32FilePos = pstFileDesc->stFileHeader.u32FileLen;
        }
        break;
        default:
            return -1;
    }

    return 0;
}
s32 SS_LWFS_Close(s32 s32Fd)
{
    SS_LWFS_FileDesc_t *pstFileDesc = NULL;
    SS_LWFS_FileDesc_t *pstFileDescN = NULL;
    SS_LWFS_FileDesc_t *pstFileDescM = NULL;
    SS_LWFS_FileDataDesc_t *pos = NULL, *posN = NULL;
    struct list_head stHead;

    pstFileDesc = (SS_LWFS_FileDesc_t *)s32Fd;
    if (!pstFileDesc)
        return -1;

    if (pstFileDesc->enFileOpt == EN_LWFS_FILE_READ_WRITE_TRUNC
        || pstFileDesc->enFileOpt == EN_LWFS_FILE_READ_WRITE)
    {
        SS_LWFS_Sync(s32Fd);// unpack.
    }
    _FileClose(pstFileDesc->s32BlockFd);
    list_add_tail(&stHead, &pstFileDesc->stList);
    list_for_each_entry_safe(pstFileDescN, pstFileDescM, &stHead, stList)
    {
        list_for_each_entry_safe(pos, posN, &pstFileDescN->stRamDataList, stRamDataList)
        {
            if (pos->pDramData)
            {
                PRINT("Free dram data %p\n", pos->pDramData);
                free(pos->pDramData);
            }
            list_del(&pos->stRamDataList);
            PRINT("Free dram node %p\n", pos);
            free(pos);
        }
        list_del(&pstFileDescN->stList);
        PRINT("Free file node %p\n", pstFileDescN);
        free(pstFileDescN);
    }
    return 0;
}


