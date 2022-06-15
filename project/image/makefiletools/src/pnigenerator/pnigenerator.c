#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define UNFD_LOGI_PART    0x8000 // bit-or if the partition needs Wear-Leveling
#define UNFD_HIDDEN_PART  0x4000 // bit-or if this partition is hidden, normally it is set for the LOGI PARTs.

#define UNFD_PART_IPL_CUST      0x01
#define UNFD_PART_BOOTLOGO      0x02
#define UNFD_PART_IPL           0x03
//#define UNFD_PART_OS            0x04
#define UNFD_PART_CUS           0x05
#define UNFD_PART_UBOOT         0x06
#define UNFD_PART_SECINFO       0x07
#define UNFD_PART_OTP           0x08
#define UNFD_PART_RECOVERY      0x09
#define UNFD_PART_E2PBAK        0x0A
#define UNFD_PART_NVRAMBAK      0x0B
#define UNFD_PART_NPT           0x0C
#define UNFD_PART_ENV           0x0D
#define UNFD_PART_MISC          0x0E

#define UNFD_PART_RTOS          0x10
#define UNFD_PART_RTOS_BAK      0X11
#define UNFD_PART_KERNEL        0x12
#define UNFD_PART_KERNEL_BAK    0x13

#define UNFD_PART_CIS           0x20
#define UNFD_PART_UBI           0x21
#define UNFD_PART_ROOTFS        0x0F
#define UNFD_PART_ROOTFS_BAK    0x1F

#define UNFD_PART_CUST0         0x30
#define UNFD_PART_CUST1         0x31
#define UNFD_PART_CUST2         0x32
#define UNFD_PART_CUST3         0x33
#define UNFD_PART_CUSTf         0x3F

#define UNFD_PART_KEY_CUST      0x40

#define UNFD_PART_END           0xC000
#define UNFD_PART_UNKNOWN       0xF000

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct
{
    u16 u16_StartBlk;		// the start block index
    u16 u16_BlkCnt;			// project team defined
    u16 u16_PartType;		// project team defined
    u16 u16_BackupBlkCnt;	// reserved good blocks count for backup, UNFD internal use.
    // e.g. u16BackupBlkCnt  = u16BlkCnt * 0.03 + 2
} SPI_NAND_PARTITION_RECORD_t;

typedef struct
{
    u32 u32_ChkSum;
    u16	u16_SpareByteCnt;
    u16	u16_PageByteCnt;
    u16	u16_BlkPageCnt;
    u16	u16_BlkCnt;
    u16 u16_PartCnt;
    u16 u16_UnitByteCnt;
    SPI_NAND_PARTITION_RECORD_t records[62];
} SpinandPartitionInfo_t;
static u32 blk_size = 0x20000;
typedef struct
{
    char asName[32];
}stPartNamesList;
static stPartNamesList astNames[62];
static u32 _checkSum(u8 *pData, u16 nByteCnt)
{
    u32 nSum = 0;

    while(nByteCnt--)
        nSum += *pData++;

    return nSum;
}
static const char* FindNameByType(u32 u32Type)
{
    switch (u32Type)
    {
        case UNFD_PART_CIS:
            return "CIS";
        case UNFD_PART_IPL:
            return "IPL";
        case UNFD_PART_MISC:
            return "MISC";
        case UNFD_PART_IPL_CUST:
            return "IPL_CUST";
        case UNFD_PART_KERNEL:
            return "KERNEL";
        case UNFD_PART_UBOOT:
            return "UBOOT";
        case UNFD_PART_RECOVERY:
            return "RECOVERY";
        case UNFD_PART_ENV:
            return "ENV";
        case UNFD_PART_RTOS:
            return "RTOS";
        case UNFD_PART_RTOS_BAK:
            return "RTOS_BAK";
        case UNFD_PART_CUST0:
            return "CUST0";
        case UNFD_PART_CUST1:
            return "CUST1";
        case UNFD_PART_CUST2:
            return "CUST2";
        case UNFD_PART_CUST3:
            return "CUST3";
        case UNFD_PART_ROOTFS:
            return "ROOTFS";
        case UNFD_PART_ROOTFS_BAK:
            return "ROOTFS_BAK";
        case UNFD_PART_END:
            return "PART_END";
        case UNFD_PART_BOOTLOGO:
            return "LOGO";
        case UNFD_PART_UBI:
            return "UBI";
        case UNFD_PART_KEY_CUST:
            return "KEY_CUST";
        default:
            return "Unknown";
    }
}
static u16 FindPartTypeByName(const char* partName)
{
     if(strcmp(partName, "CIS") == 0 )
         return UNFD_PART_CIS;
     if(strcmp(partName, "MISC") == 0 )
         return UNFD_PART_MISC;
     else if(strcmp(partName, "IPL_CUST0") == 0 || strcmp(partName, "IPL_CUST1") == 0 || strcmp(partName, "IPL_CUST2") == 0)
         return UNFD_PART_IPL_CUST;
     else if(strcmp(partName, "KERNEL") ==0 )
         return UNFD_PART_KERNEL;
     else if((strcmp(partName, "IPL0") == 0 ) || (strcmp(partName, "IPL1") == 0 ) || strcmp(partName, "IPL2") == 0 )
         return UNFD_PART_IPL;
     else if((strcmp(partName, "UBOOT0") == 0 ) || (strcmp(partName, "UBOOT1") == 0 ))
         return UNFD_PART_UBOOT;
     else if(strcmp(partName, "RECOVERY") == 0 )
         return UNFD_PART_RECOVERY;
     else if((strcmp(partName, "ENV") == 0 ) || (strcmp(partName, "ENV0") == 0 ) || (strcmp(partName, "ENV1") == 0 ))
         return UNFD_PART_ENV;
     else if(strcmp(partName, "RTOS") == 0 )
         return UNFD_PART_RTOS;
     else if(strcmp(partName, "RTOS_BACKUP") == 0 )
         return UNFD_PART_RTOS_BAK;
     else if(strcmp(partName, "rootfs") == 0 )
         return UNFD_PART_ROOTFS;
     else if(strcmp(partName, "rootfs_bak") == 0 )
         return UNFD_PART_ROOTFS_BAK;
     else if(strcmp(partName, "CUST0") == 0 )
         return UNFD_PART_CUST0;
     else if(strcmp(partName, "CUST1") == 0 )
         return UNFD_PART_CUST1;
     else if(strcmp(partName, "CUST2") == 0 )
         return UNFD_PART_CUST2;
     else if(strcmp(partName, "CUST3") == 0 )
         return UNFD_PART_CUST3;
     else if(strcmp(partName, "CUSTf") == 0 )
         return UNFD_PART_CUSTf;
     else if(strcmp(partName, "UBI") == 0 )
         return UNFD_PART_UBI;
     else if(strcmp(partName, "PART_END") == 0 )
         return UNFD_PART_END;
     else if(strcmp(partName, "LOGO") == 0)
         return UNFD_PART_BOOTLOGO;
     else if(strcmp(partName, "KEY_CUST") == 0)
         return UNFD_PART_KEY_CUST;
     else
         return UNFD_PART_UNKNOWN;
}
static u32 parse_partition_section(const char *section, u32 start_partion_addr, void *usr_data, u32 *p_i)
{
    char partition_name[30];
    u32 partition_size = 0;
    u32 partition_bkcnt = 0;
    char *temp_pos = NULL;
    u32 start_addr = 0;
    SpinandPartitionInfo_t *pstPni = (SpinandPartitionInfo_t *)usr_data;

    if (*p_i == 62)
        return start_partion_addr;

    start_addr = start_partion_addr;
    if (*section == '-')
    {
        partition_size = pstPni->u16_BlkCnt * blk_size - start_addr;
        section += 2;
        temp_pos = strstr(section, ")");
        if (temp_pos != NULL)
        {
            *temp_pos = 0;
        }
        strcpy(partition_name, section);
    }
    else
    {
        temp_pos = strstr(section, "@");
        if (temp_pos)
        {
            temp_pos++;
            sscanf(section, "0x%x@0x%x(%s)", &partition_size, &start_addr, partition_name);
            //printf("SIZE 0x%x ADDR 0x%x Name %s\n", partition_size, start_addr, partition_name);
        }
        else
        {
            sscanf(section, "0x%x(%s)", &partition_size, partition_name);
        }
        temp_pos = strstr(partition_name, ")");
        if (temp_pos != NULL)
        {
            *temp_pos = 0;
            temp_pos++;
            sscanf(temp_pos, "%d", &partition_bkcnt);
        }
    }
    if (start_addr >= pstPni->u16_BlkCnt * blk_size)
    {
        return start_addr;
    }
    pstPni->records[*p_i].u16_StartBlk = start_addr / blk_size;
    pstPni->records[*p_i].u16_BlkCnt = partition_size / blk_size;
    pstPni->records[*p_i].u16_PartType = FindPartTypeByName(partition_name);
    pstPni->records[*p_i].u16_BackupBlkCnt = partition_bkcnt;
    strcpy(astNames[*p_i].asName, partition_name);
    //printf("name: %s start 0x%x size 0x%x idx %d\n",partition_name, start_partion_addr, partition_size, *p_i);
    (*p_i)++;

    return partition_size + partition_bkcnt * blk_size + start_addr;
}
static u32 parse_partition(const char *string, u32 start_partion_addr, void *usr_data, u32 *p_i)
{
    char partition_section[50];
    const char *p_section_start = NULL;
    const char *p_section_end = NULL;

    if (!string)
    {
        return start_partion_addr;
    }
    p_section_start = string;
    while(1)
    {
        memset(partition_section, 0, 50);
        p_section_end = strstr(p_section_start, ",");
        if (p_section_end == NULL)
        {
            strcpy(partition_section, p_section_start);
            start_partion_addr = parse_partition_section(partition_section, start_partion_addr, usr_data, p_i);
            break;
        }
        strncpy(partition_section, p_section_start, p_section_end - p_section_start);
        start_partion_addr = parse_partition_section(partition_section, start_partion_addr, usr_data, p_i);
        p_section_start = p_section_end + 1;
    }

    return  start_partion_addr;
}
u32 easy_atoi(const char *pStr)
{
    u32 intStrLen = strlen(pStr);
    u16 bUseHex = 0;
    u32 intRetNumber = 0;

    if (pStr == NULL)
    {
        return 0xFFFFFFFF;
    }

    if (intStrLen > 2)
    {
        if (pStr[0] == '0' &&(pStr[1] == 'X' || pStr[1] == 'x'))
        {
            bUseHex = 1;
            pStr += 2;
        }
    }
    if (bUseHex == 1)
    {
        for (int i = 0; i < intStrLen - 2; i++)
        {
            if ((pStr[i] > '9' || pStr[i] < '0')    \
                && (pStr[i] > 'f' || pStr[i] < 'a') \
                && (pStr[i] > 'F' || pStr[i] < 'A'))
            {
                return 0xFFFFFFFF;
            }
        }
        sscanf(pStr, "%x", &intRetNumber);
    }
    else
    {
        for (int i = 0; i < intStrLen; i++)
        {
            if (pStr[i] > '9' || pStr[i] < '0')
            {
                return 0xFFFFFFFF;
            }
        }
        intRetNumber =  atoi(pStr);
    }
    return intRetNumber;
}
int main(char argc, char **argv)
{
    SpinandPartitionInfo_t stPni;
    SpinandPartitionInfo_t stPniBoot;
    u32 i = 0;
    int fd = 0;
    int result;
    u8 is_write = 0;
    u8 is_read = 0;
    char *file_name = NULL;
    char *boot_part = NULL;
    char *sys_part = NULL;
    u32 start_partion_addr = 0;

    memset(&stPni, 0, sizeof(SpinandPartitionInfo_t));
    memset(&stPniBoot, 0, sizeof(SpinandPartitionInfo_t));
    memset(&astNames, 0, sizeof(stPartNamesList) * 62);
    stPniBoot.u16_SpareByteCnt = 64;
    stPniBoot.u16_PageByteCnt = 2048;
    stPniBoot.u16_BlkPageCnt = 64;
    stPniBoot.u16_BlkCnt = 1024;
    stPniBoot.u16_UnitByteCnt = 8;

    while ((result = getopt(argc, argv, "s:p:b:k:u:l:t:y:r:o:")) != -1)
    {
        switch (result)
        {
            case 's':
            {
                stPniBoot.u16_SpareByteCnt = (u16)easy_atoi(optarg);
                is_write = 1;
            }
            break;
            case 'p':
            {
                stPniBoot.u16_PageByteCnt = (u16)easy_atoi(optarg);
                is_write = 1;
            }
            break;
            case 'b':
            {
                stPniBoot.u16_BlkPageCnt = (u16)easy_atoi(optarg);
                is_write = 1;
            }
            break;
            case 'k':
            {
                stPniBoot.u16_BlkCnt = (u16)easy_atoi(optarg);
                is_write = 1;
            }
            break;
            case 'u':
            {
                stPniBoot.u16_UnitByteCnt = easy_atoi(optarg);;
                is_write = 1;
            }
            break;
            case 'l':
            {
                blk_size = easy_atoi(optarg);
            }
            break;
            case 't':
            {
                if (strlen(optarg) != 0)
                {
                    boot_part = (char *)malloc(strlen(optarg) + 1);
                    assert(boot_part);
                    strcpy(boot_part, optarg);
                    printf("BOOT: %s\n", boot_part);
                    is_write = 1;
                }
            }
            break;
            case 'y':
            {
                if (strlen(optarg) != 0)
                {
                    sys_part = (char *)malloc(strlen(optarg) + 1);
                    assert(sys_part);
                    strcpy(sys_part, optarg);
                    printf("SYS: %s\n", sys_part);
                    is_write = 1;
                }
            }
            break;
            case 'r':
            {

                file_name = (char *)malloc(strlen(optarg) + 1);
                assert(file_name);
                strcpy(file_name, optarg);
                is_read = 1;
            }
            break;
            case 'o':
            {
                file_name = (char *)malloc(strlen(optarg) + 1);
                assert(file_name);
                strcpy(file_name, optarg);
                is_write = 1;
            }
            break;
            default:
                break;
        }
    }
    if (is_read && is_write && (!strlen(file_name)))
    {
        printf("ERROR OPT!\n");

        goto ERR_NOFD;
    }
    if (is_write && file_name)
    {
        fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0777);
        if (-1 == fd)
        {
            perror("open");
            goto ERR_NOFD;
        }
        start_partion_addr = parse_partition(boot_part, start_partion_addr, (void *)&stPniBoot, &i);
        stPniBoot.u16_PartCnt = i;
        stPniBoot.u32_ChkSum = _checkSum((u8*)&(stPniBoot.u16_SpareByteCnt), 0x200 - 0x04);
        write(fd, &stPniBoot, sizeof(SpinandPartitionInfo_t));
        /*Sys part is no need to write in pni, only to print on screen.*/
        memcpy(&stPni, &stPniBoot, sizeof(SpinandPartitionInfo_t));
        start_partion_addr = parse_partition(sys_part, start_partion_addr, (void *)&stPni, &i);
        stPni.u16_PartCnt = i;
        stPni.u32_ChkSum = _checkSum((u8*)&(stPni.u16_SpareByteCnt), 0x200 - 0x04);
        printf("FLASH HAS USED 0x%xKB\n", start_partion_addr);
    }
    else if (is_read && file_name)
    {
        fd = open(file_name, O_RDONLY);
        if (-1 == fd)
        {
            perror("open");
            goto ERR_NOFD;
        }
        read(fd, &stPni, sizeof(SpinandPartitionInfo_t));
    }
    printf("ChkSum       : %d\n", stPni.u32_ChkSum);
    printf("SpareByteCnt : %d\n", stPni.u16_SpareByteCnt);
    printf("PageByteCnt  : %d\n", stPni.u16_PageByteCnt);
    printf("BlkPageCnt   : %d\n", stPni.u16_BlkPageCnt);
    printf("BlkCnt       : %d\n", stPni.u16_BlkCnt);
    printf("PartCnt      : %d\n", stPni.u16_PartCnt);
    printf("UnitByteCnt  : %d\n", stPni.u16_UnitByteCnt);
    if (_checkSum((u8*)&(stPni.u16_SpareByteCnt), 0x200 - 0x04) == stPni.u32_ChkSum)
    {
        printf("Checksum ok!!\n");
    }
    else
    {
        printf("Checksum error!\n");

        goto ERR;
    }
    printf("IDX: %17s %17s %17s %16s\n","StartBlk:", "BlkCnt:", "BackupBlkCnt:", "PartType:");
    for (i = 0; i < stPni.u16_PartCnt; i++)
    {
        if (strlen(astNames[i].asName) == 0)
            strcpy(astNames[i].asName, FindNameByType(stPni.records[i].u16_PartType));
        printf("%3d: %4d,(%#010X) %4d,(%#010X) %4d,(%#010X)  %#06X,(%s)\n", i,
                stPni.records[i].u16_StartBlk, (stPni.records[i].u16_StartBlk * blk_size), stPni.records[i].u16_BlkCnt, (stPni.records[i].u16_BlkCnt * blk_size),
                stPni.records[i].u16_BackupBlkCnt, (stPni.records[i].u16_BackupBlkCnt * blk_size), stPni.records[i].u16_PartType, astNames[i].asName);
    }

ERR:
    close(fd);
ERR_NOFD:
    if (boot_part)
        free(boot_part);
    if (sys_part)
        free(sys_part);
    if (file_name)
        free(file_name);

    return 0;
}
