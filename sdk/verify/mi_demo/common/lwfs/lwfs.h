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

#ifndef __LWFS__
#define __LWFS__

typedef char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
#define LWFS_PARTITION_HEADER_SIZE 64
#define LWFS_FILE_HEADER_SIZE 64
#define LWFS_MAGIC_ID 0x5346574C
#define LWFS_HEADER_VER 1

#ifndef min
#define min(a, b) (a) < (b)?(a):(b)
#endif
#ifndef max
#define max(a, b) (a) > (b)?(a):(b)
#endif

typedef enum
{
    EN_LWFS_FILE_READ_WRITE_TRUNC,
    EN_LWFS_FILE_READ_ONLY,
    EN_LWFS_FILE_READ_WRITE
}SS_LWFS_FileOpertion_e;
#define EN_LWFS_SEEK_SET	0	/* seek relative to beginning of file */
#define EN_LWFS_SEEK_CUR	1	/* seek relative to current file position */
#define EN_LWFS_SEEK_END	2	/* seek relative to end of file */

typedef struct SS_LWFS_PartitionHeaderData_s
{
    u32 u32MagicId;
    u32 u32HeaderVer;
    u32 u32HeaderSize; /*File header size + partition header size*/
    u32 u32FileAlignement;
    u32 u32FileCount;
}SS_LWFS_PartitionHeaderData_t;

typedef struct SS_LWFS_FileHeaderData_s
{
    s8 s8FileName[32];
    u32 u32FilePosition;
    u32 u32FileLen;
    u32 u32FilePaddingLen;
    u32 u32CrcCheck;
    u32 u32CompressType;
}SS_LWFS_FileHeaderData_t;

typedef union
{
    u8 u8Align[LWFS_PARTITION_HEADER_SIZE];
    SS_LWFS_PartitionHeaderData_t stData;
}SS_LWFS_PartitionHeaderData_u;

typedef union
{
    u8 u8Align[LWFS_FILE_HEADER_SIZE];
    SS_LWFS_FileHeaderData_t stData;
}SS_LWFS_FileHeaderData_u;

s32 SS_LWFS_List(const s8 *s8MtdPath);
s32 SS_LWFS_Open(const s8 *s8MtdPath, const s8 *s8FileName, SS_LWFS_FileOpertion_e);
s32 SS_LWFS_Read(s32 s32Fd, void *buf, s32 size);
s32 SS_LWFS_Write(s32 s32Fd, void *buf, s32 size);
s32 SS_LWFS_Sync(s32 s32Fd);
s32 SS_LWFS_Seek(s32 s32Fd, s32 s32Offset, s32 s32Whence);
s32 SS_LWFS_Close(s32 s32FileFd);

#endif //LWFS
