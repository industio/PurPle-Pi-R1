#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "es7243.h"
#include "i2c.h"

unsigned char u8Es7243InitSetting[][2] = {
    {0x00, 0x09},
    {0x06, 0x18},
    {0x01, 0x0c},
    {0x02, 0x10},
    {0x03, 0x04},
    {0x04, 0x01},
    {0x05, 0x1a},
    {0x09, 0x3F},
    {0x05, 0x13},
    {0x08, 0x43},
    {0x06, 0x00},
};

unsigned char u8Es7243DeinitSetting[][2] = {
    {0x06, 0x05},
    {0x05, 0x1B},
    {0x06, 0x18},
    {0x07, 0x3F},
    {0x08, 0x4B},
    {0x09, 0x9F},
};

int ES7243_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES7243_CHIP_ADDR);
}

int ES7243_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES7243_CHIP_ADDR);
}

int ES7243_Init(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    s32Ret = I2C_Init(10, 50);
    if (0 != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = ES7243_ReadByte(0x0e, &val);
    if (0 != s32Ret)
    {
        return s32Ret;
    }
    printf("=============== 7243 Chip ID : 0x%x ======================.\n", val);

    for (u32Index = 0; u32Index < (sizeof(u8Es7243InitSetting) / sizeof(u8Es7243InitSetting[0])); u32Index++)
    {
        reg = u8Es7243InitSetting[u32Index][0];
        val = u8Es7243InitSetting[u32Index][1];

        s32Ret = ES7243_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }
    printf("================Init ES7243 success.======================\n");

    return 0;
}

int ES7243_Deinit(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    s32Ret = I2C_Init(10, 5);
    if (0 != s32Ret)
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < (sizeof(u8Es7243DeinitSetting) / sizeof(u8Es7243DeinitSetting[0])); u32Index++)
    {
        reg = u8Es7243DeinitSetting[u32Index][0];
        val = u8Es7243DeinitSetting[u32Index][1];
        s32Ret = ES7243_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }

	printf("Suspend ES7243 success.\n");
	return 0;
}

