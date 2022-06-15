#include <stdio.h>
#include <string.h>

#include "mmapinfo.h"

unsigned int gCpuPhyAddr0 = 0;
unsigned int gCpuPhyAddr1 = 0;
int main(char argc, char **argv)
{
    MMAPInfo info;
	int i;

    if (argc != 5)
    {
        printf("Error input!\n");

        return -1;
    }
    memset(&info, 0, sizeof(MMAPInfo));
	if (!strcasecmp(argv[2], "K6"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0x60000000;
	}
	else if (!strcasecmp(argv[2], "K6L"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0x60000000;
	}
	else if (!strcasecmp(argv[2], "I2"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0x60000000;
	}
	else if (!strcasecmp(argv[2], "I5"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0xA0000000;
	}
	else if (!strcasecmp(argv[2], "I6"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0xA0000000;
	}
	else if (!strcasecmp(argv[2], "I2M"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0xA0000000;
	}
	else if (!strcasecmp(argv[2], "I6E"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0xA0000000;
	}
	else if (!strcasecmp(argv[2], "I6B0"))
	{
		gCpuPhyAddr0 = 0x20000000;
		gCpuPhyAddr1 = 0xA0000000;
	}
	else
	{
		printf("Not support chip");
		return -1;
	}
	ParserMmap(argv[1], &info);
#if 0
	printf("total:%x\n", info.header->total_mem_size);
	printf("miu0:%x\n", info.header->miu0_mem_size);
	printf("miu1:%x\n", info.header->miu1_mem_size);
	printf("miu2:%x\n", info.header->miu2_mem_size);
	printf("miub:%x\n", info.header->miu_boundary);
	printf("miub2:%x\n", info.header->miu_boundary2);
	printf("item:%d\n", info.header->n_mmap_items);
	printf("4k:%x\n", info.header->b_is_4k_align);
	for(i = 0; i < info.header->n_mmap_items; ++i)
	{
		printf("name:%s\n", info.name[i]);
		printf("gid:%x\n", info.context[i].u32gID);
		printf("addr:%x\n", info.context[i].u32Addr);
		printf("size:%x\n", info.context[i].u32Size);
		printf("layer:%u\n", info.context[i].u8Layer);
		printf("align:%x\n", info.context[i].u32Align);
		printf("miuNo:%x\n", info.context[i].u32MiuNo);
		printf("cmahid:%x\n", info.context[i].u32CMAHid);
	}
#endif
	for(i = 0; i < info.header->n_mmap_items; ++i)
	{
		if (!strcmp(info.name[i], argv[3]))
		{
			if (!strcmp("miuaddr", argv[4]))
			{
				printf("0x%x", info.context[i].u32Addr);
			}
			if (!strcmp("phyaddr", argv[4]))
			{
				if (info.context[i].u32MiuNo == 0)
				{
					printf("0x%x", info.context[i].u32Addr + gCpuPhyAddr0);
				}
				else if (info.context[i].u32MiuNo == 1)
				{
					printf("0x%x", info.context[i].u32Addr - info.header->miu_boundary + gCpuPhyAddr1);
				}
				else
				{
					printf("Not support miu");
					
					return -1;
				}
			}
			if (!strcmp("size", argv[4]))
			{
				printf("0x%x", info.context[i].u32Size);
			}
			if (!strcmp("miu", argv[4]))
			{
				printf("%d", info.context[i].u32MiuNo);
			}
		}
	}

	return 0;
}
