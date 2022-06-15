#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
//#include <strings.h>

/*
 * How to update NOR flash layout:
 * 1. Modify mxp_gen.c for change layout
 * 2. Build: gcc mxp_gen.c -o mxp_gen
 * 3. Generate MXP_SF.bin: ./mxp_gen MXP_SF.bin
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define MAX_RECORD_PRINT_COUNT 32

typedef struct {
    u8    	magic_prefix[4];//MXPT
    u8      version;
    u8  	type; //0X00:tag 0xFF:terminated 0x01:general
    u8  	format;
    u8    padding;
    u64		start;
    u64		size;
    u32		block;
    u32		block_count;
    u8	    name[16]; //should be 0 terminated
    u8		backup[16]; //should be 0 terminated
    u8		hash[32];
    u8      reserved[24];
    u32		crc32;
    u8		status;
    u8 		magic_suffix[4];//TPXM
    u8      reserved2[7];
} mxp_record;


#define MXP_PART_MAGIC_PREFIX			"MXPT"
#define MXP_PART_MAGIC_SUFFIX			"TPXM"

#define MXP_STORAGE_TYPE_FLASH       	0x01

#define MXP_PART_TYPE_TAG 				0x00
#define MXP_PART_TYPE_NORMAL			0x01
#define MXP_PART_TYPE_MTD 			    0x02

#define MXP_PART_FORMAT_NONE 			0x00


#define MXP_PART_STATUS_UPGRADING		0xF7
#define MXP_PART_STATUS_READY			0xF6
#define MXP_PART_STATUS_ACTIVE			0xF5
#define MXP_PART_STATUS_INACTIVE		0xF4
#define MXP_PART_STATUS_EMPTY			0x00

#define TABLE_SIZE 4096
#define COUNT (TABLE_SIZE/sizeof(mxp_record))

mxp_record mxpt[COUNT];
void out_record(const char* name, u8 type, u32 start, u32 size, u8 status,u8 index)
{

    strcpy(mxpt[index].name,name);
    mxpt[index].type=type;
    mxpt[index].start=start;
    mxpt[index].size=size;
    mxpt[index].status=status;
    mxpt[index].version=0x01;
}

static void print_mxp_record(int index,mxp_record* rc)
{
    printf("[mxp_record]: %d\n",index);
    printf("     name: %s\n",rc->name);
    printf("     type: 0x%02X\n",rc->type);
    printf("   format: 0x%02X\n",rc->format);
    printf("   backup: %s\n",( ((char)0xFF)==((char)rc->backup[0]) )?(""):((char *)rc->backup));
    printf("    start: 0x%08X\n",(unsigned int)rc->start);
    printf("     size: 0x%08X\n",(unsigned int)rc->size);
    printf("   status: 0x%02X\n",rc->status);
    printf("\n");

}
static u32 parse_partition_section(const char *section, u32 start_partion_addr, u32 usr_data, u8 *p_i)
{
    char partition_name[30];
    u32 partition_size = 0;
    char *temp_pos = NULL;

    if (*p_i >= COUNT)
    {
        printf("ERROR!! partition count over range.\n");

        return start_partion_addr;
    }
    sscanf(section, "0x%x(%s)", &partition_size, partition_name);
    temp_pos = strstr(partition_name, ")");
    if (temp_pos != NULL)
    {
        *temp_pos = 0;
    }
    //printf("name: %s start 0x%x size 0x%x usr %d idx %d\n",partition_name, start_partion_addr, partition_size, usr_data, *p_i);
    out_record(partition_name, usr_data, start_partion_addr, partition_size, MXP_PART_STATUS_EMPTY, *p_i);
    (*p_i)++;

    return partition_size + start_partion_addr;
}

static u32 parse_partition(const char *string, u32 start_partion_addr, u32 usr_data, u8 *p_i)
{
    char partition_section[50];
    const char *p_section_start = NULL;
    const char *p_section_end = NULL;

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
static void parse(const char *boot_conf, const char *sys_conf)
{
    u32 start_partion_addr = 0;
    u8 i = 0;

    start_partion_addr = parse_partition(boot_conf, start_partion_addr, MXP_PART_TYPE_NORMAL, &i);
    out_record("BOOT", (MXP_PART_TYPE_NORMAL|MXP_PART_TYPE_MTD), 0x00000000, start_partion_addr, MXP_PART_STATUS_EMPTY, i++);
    start_partion_addr = parse_partition(sys_conf, start_partion_addr, MXP_PART_TYPE_NORMAL|MXP_PART_TYPE_MTD, &i);
    printf("NOR FLASH HAS USED 0x%xKB\n", start_partion_addr);
}

static void env_merge(void)
{
    int i;
    for(i = 0; i < COUNT; i++)
    {
        if(0 == strcmp(mxpt[i].name, "UBOOT_ENV"))
        {
            if(i != 0)
            {
                mxpt[i-1].size += mxpt[i].size;

                mxpt[i].type &= ~MXP_PART_TYPE_MTD;
            }
        }


    }
}

int main(int argc, char *argv[])
{

    ssize_t nrd;
    int fd;

    int i=0;
    printf(":[%d]\n",__LINE__);

    if (argc != 3 && argc != 4)
    {
         printf("ERROR! argc is %d\n", argc);

         return -1;
    }

    memset(mxpt,0xFF,TABLE_SIZE);
    //printf("sizeof(mxp_record):%u\n",sizeof(mxp_record));
    if(0==strcmp(argv[1],"-i"))
    {
        int i=0;
        fd = open(argv[2], O_RDONLY);
        read(fd,mxpt,TABLE_SIZE);
        for(i=0;i<MAX_RECORD_PRINT_COUNT;i++)
        {
            if(MXP_PART_TYPE_TAG==mxpt[i].type)
            {
                break;
            }
            print_mxp_record(i,&mxpt[i]);
        }
        printf("Available MXP record count:%d\n",i);
        close(fd);
        return 0;
    }

    printf("BOOT: %s\n", argv[1]);
    printf("SYS: %s\n", argv[2]);
    printf("MXP count max %lu\n", COUNT);
    fd = open(argv[3], O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (-1 == fd)
    {
        perror("open");
        return -1;
    }

    //initiailize table
    for(i=0;i<COUNT;i++)
    {
        memcpy(mxpt[i].magic_prefix,"MXPT",4);
        memcpy(mxpt[i].magic_suffix,"TPXM",4);
        mxpt[i].type=MXP_PART_TYPE_TAG;
        memset(mxpt[i].name,0x00,16);
        memset(mxpt[i].backup,0x00,16);
        mxpt[i].format=MXP_PART_FORMAT_NONE;
        mxpt[i].status=MXP_PART_STATUS_EMPTY;
        mxpt[i].version=0x01;
    }
    parse(argv[1], argv[2]);
    env_merge();
    write(fd,mxpt,TABLE_SIZE);
    close(fd);

    //dump
    for(i=0;i<MAX_RECORD_PRINT_COUNT;i++)
    {
        if(MXP_PART_TYPE_TAG==mxpt[i].type)
        {
        break;
        }

        printf("%12s:",mxpt[i].name);
        if((unsigned int)mxpt[i].size != 0xFFFFFFFF)
        {
            printf(" 0x%08X-0x%08X",(unsigned int)mxpt[i].start, (unsigned int)mxpt[i].start+(unsigned int)mxpt[i].size);
            printf(" size:%dKB",(unsigned int)mxpt[i].size/1024);
        }
        else
        {
            printf(" 0x%08X-0x%08X",(unsigned int)mxpt[i].start, 0xFFFFFFFF);
            printf(" size:-%dKB",(unsigned int)mxpt[i].start/1024);
        }
        printf("\n");

    }
    printf("Available MXP record count:%d\n",i);


    return 0;
}
