#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "es8156.h"
#include "i2c.h"

unsigned char u8Es8156InitSetting[][2] = {
    {0x02, 0x84},
    {0x00, 0x3c},
    {0x00, 0x1c},
    {0x02, 0x84},
    {0x0A, 0x01},
    {0x0B, 0x01},
    {0x0D ,0x14},
    {0x01, 0xC1},
    {0x18, 0x00},
    {0x08, 0x3F},
    {0x09, 0x02},
    {0x00, 0x01},
    {0x22, 0x00},
    {0x23, 0xCA},
    {0x24, 0x00},
    {0x25, 0x20},
    {0x14, 0xbf},
};

unsigned char u8Es8156DeinitSetting[][2] = {
    {0x14, 0x00},
    {0x19, 0x02},
    {0x21, 0x1F},
    {0x22, 0x02},
    {0x25, 0x21},
    {0x25, 0x01},
    {0x25, 0x87},
    {0x18, 0x01},
    {0x09, 0x02},
    {0x09, 0x01},
    {0x08, 0x00},
};


int ES8156_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES8156_CHIP_ADDR);
}

int ES8156_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES8156_CHIP_ADDR);
}

int ES8156_Init(void)
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

    s32Ret = ES8156_ReadByte(0xfe, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID0:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    s32Ret = ES8156_ReadByte(0xfd, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID1:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156InitSetting)) / (sizeof(u8Es8156InitSetting[0]))); u32Index++)
    {
        reg = u8Es8156InitSetting[u32Index][0];
        val = u8Es8156InitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }

    printf("======================Init ES8156 success.========================\n");

    return 0;
}

int ES8156_Deinit(void)
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

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156DeinitSetting)) / (sizeof(u8Es8156DeinitSetting[0]))); u32Index++)
    {
        reg = u8Es8156DeinitSetting[u32Index][0];
        val = u8Es8156DeinitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }

	printf("Deinit ES8156 success.\n");
    return 0;
}
