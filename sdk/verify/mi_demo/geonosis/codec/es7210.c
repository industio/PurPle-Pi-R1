#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "es7210.h"
#include "i2c.h"

unsigned char u8Es7210_2AmicInitSetting[][2] = {
    {0x00, 0xFF},
    {0x00, 0x32},
    {0x09, 0x30},
    {0x0A, 0x30},
    {0x23, 0x2A},
    {0x22, 0x0A},
    {0x21, 0x2A},
    {0x20, 0x0A},
    {0x40, 0xC3},
    {0x41, 0x70},
    {0x42, 0x70},
    {0x43, 0x10},
    {0x44, 0x10},
    {0x45, 0x10},
    {0x46, 0x10},
    {0x47, 0x08},
    {0x48, 0x08},
    {0x49, 0x08},
    {0x4A, 0x08},
    {0x07, 0x20},
    {0x02, 0x41},
    {0x4B, 0x0F},
    {0x4C, 0x0F},
    {0x02, 0x41},
    {0x06, 0x00},
    {0x08, 0x12},
    {0x11, 0x60},
    {0x43, 0x1e},
    {0x44, 0x1e},
    {0x45, 0x10},
    {0x46, 0x10},
    {0x00, 0x71},
    {0x00, 0x41},
/*
    {0x1b, 0xff},
    {0x1c, 0xff},
    {0x1d, 0xbf},
    {0x1e, 0xbf},
*/
};

unsigned char u8Es7210_2DmicInitSetting[][2] = {
    {0x00, 0xFF},
    {0x00, 0x32},
    {0x09, 0x30},
    {0x0A, 0x30},
    {0x23, 0x2A},
    {0x22, 0x0A},
    {0x21, 0x2A},
    {0x20, 0x0A},
    {0x40, 0xC3},
    {0x41, 0x70},
    {0x42, 0x70},
    {0x43, 0x10},
    {0x44, 0x10},
    {0x45, 0x10},
    {0x46, 0x10},
    {0x47, 0x08},
    {0x48, 0x08},
    {0x49, 0x08},
    {0x4A, 0x08},
    {0x07, 0x20},
    {0x02, 0x41},
    {0x4B, 0x0F},
    {0x4C, 0x0F},
    {0x02, 0x41},
    {0x06, 0x00},
    {0x08, 0x12},
    {0x11, 0x60},
/*
    {0x43, 0x1e},
    {0x44, 0x1e},
    {0x45, 0x12},
    {0x46, 0x10},
*/
    {0x00, 0x71},
    {0x00, 0x41},
    {0x10, 0xc0},
/*
    {0x1b, 0xff},
    {0x1c, 0xff},
*/
    {0x1d, 0xff},
    {0x1e, 0xff},
};

unsigned char u8Es7210_4AmicInitSetting[][2] = {
    {0x00, 0xFF},
    {0x00, 0x32},
    {0x09, 0x30},
    {0x0A, 0x30},
    {0x23, 0x2A},
    {0x22, 0x0A},
    {0x21, 0x2A},
    {0x20, 0x0A},
    {0x40, 0xC3},
    {0x41, 0x70},
    {0x42, 0x70},
    {0x43, 0x10},
    {0x44, 0x10},
    {0x45, 0x10},
    {0x46, 0x10},
    {0x47, 0x08},
    {0x48, 0x08},
    {0x49, 0x08},
    {0x4A, 0x08},
    {0x07, 0x20},
    {0x02, 0x41},
    {0x4B, 0x0F},
    {0x4C, 0x0F},
    {0x02, 0x41},
    {0x06, 0x00},
    {0x08, 0x20},
    {0x11, 0x60},
    {0x12, 0x07},
    {0x43, 0x1e},
    {0x44, 0x1e},
    {0x45, 0x12},
    {0x46, 0x10},
    {0x00, 0x71},
    {0x00, 0x41},
/*
    {0x1b, 0xff},
    {0x1c, 0xff},
    {0x1d, 0xff},
    {0x1e, 0xff},
*/
};

unsigned char u8Es7210_3AmicInitSetting[][2] = {
    {0x00, 0xFF},
    {0x00, 0x32},
    {0x09, 0x30},
    {0x0A, 0x30},
    {0x23, 0x2A},
    {0x22, 0x0A},
    {0x21, 0x2A},
    {0x40, 0xC3},
    {0x41, 0x70},
    {0x42, 0x70},
    {0x43, 0x10},
    {0x44, 0x10},
    {0x45, 0x10},
    {0x46, 0x10},
    {0x47, 0x08},
    {0x48, 0x08},
    {0x49, 0x08},
    {0x4A, 0x08},
    {0x07, 0x20},
    {0x02, 0x41},
    {0x4B, 0x0F},
    {0x4C, 0x0F},
    {0x02, 0x41},
    {0x06, 0x00},
    {0x08, 0x12},
    {0x11, 0x60},
    //0x20, mic1 mic2
    //0x10, mic3 mic4
    //mic1和mic3短接，所以无论怎么切拿到的Chn0都是mic1的数据
    {0x12, 0x10},
    {0x43, 0x1e},
    {0x44, 0x1e},
    {0x45, 0x1e},
    {0x46, 0x1e},
    {0x00, 0x71},
    {0x00, 0x41},
};

unsigned char u8Es7210NormalDeinitSetting[][2] = {
    {0x04, 0x02},
    {0x04, 0x01},
    {0xf7, 0x30},
    {0xf9, 0x01},
    {0x16, 0xff},
    {0x17, 0x00},
    {0x01, 0x38},
    {0x20, 0x00},
    {0x21, 0x00},
    {0x00, 0x00},
    {0x00, 0x1e},
    {0x01, 0x30},
    {0x01, 0x00},
};

unsigned char u8Es7210_3AmicDeinitSetting[][2] = {
    {0x06, 0x00},
    {0x4B, 0xFF},
    {0x4C, 0xFF},
    {0x0B, 0xD0},
    {0x40, 0x80},
    {0x01, 0x7F},
    {0x06, 0x07},
};

int ES7210_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES7210_CHIP_ADDR);
}

int ES7210_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES7210_CHIP_ADDR);
}

int _ES7210_Init(unsigned char initSetting[][2] , unsigned char initSettingCount)
{
    int s32Ret;
    unsigned char reg = 0;
    unsigned int u32Index = 0;
    unsigned char val = 0;

    s32Ret = I2C_Init(10, 5);
    if (0 != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = ES7210_ReadByte(0x3e, &val);
    if (0 == s32Ret)
    {
        printf("ES7210 ID0:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    s32Ret = ES7210_ReadByte(0x3d, &val);
    if (0 == s32Ret)
    {
        printf("ES7210 ID1:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < initSettingCount; u32Index++)
    {
        reg = initSetting[u32Index][0];
        val = initSetting[u32Index][1];

        s32Ret = ES7210_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }

    printf("================Init ES7210 success.======================\n");

    return 0;
}

int _ES7210_Deinit(unsigned char deinitSetting[][2], unsigned char deinitSettingCount)
{
    int s32Ret;
    unsigned char reg = 0;
    unsigned int u32Index = 0;
    unsigned char val = 0;

    s32Ret = I2C_Init(10, 5);
    if (0 != s32Ret)
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < deinitSettingCount; u32Index++)
    {
        reg = deinitSetting[u32Index][0];
        val = deinitSetting[u32Index][1];

        s32Ret = ES7210_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }

    printf("========================Suspend ES7210 success.===========================\n");
	return 0;
}

int ES7210_2AmicInit(void)
{
    return _ES7210_Init(u8Es7210_2AmicInitSetting, 
            sizeof(u8Es7210_2AmicInitSetting) / sizeof(u8Es7210_2AmicInitSetting[0]));
}

int ES7210_2AmicDeInit(void)
{
    return _ES7210_Deinit(u8Es7210NormalDeinitSetting,
            sizeof(u8Es7210NormalDeinitSetting) / sizeof(u8Es7210NormalDeinitSetting[0]));
}

int ES7210_2DmicInit(void)
{
    return _ES7210_Init(u8Es7210_2DmicInitSetting,
            sizeof(u8Es7210_2DmicInitSetting) / sizeof(u8Es7210_2DmicInitSetting[0]));
}

int ES7210_2DmicDeInit(void)
{
    return _ES7210_Deinit(u8Es7210NormalDeinitSetting,
            sizeof(u8Es7210NormalDeinitSetting) / sizeof(u8Es7210NormalDeinitSetting[0]));
}

int ES7210_4AmicInit(void)
{
    return _ES7210_Init(u8Es7210_4AmicInitSetting,
            sizeof(u8Es7210_4AmicInitSetting) / sizeof(u8Es7210_4AmicInitSetting[0]));
}

int ES7210_4AmicDeInit(void)
{
    return _ES7210_Deinit(u8Es7210NormalDeinitSetting,
            sizeof(u8Es7210NormalDeinitSetting) / sizeof(u8Es7210NormalDeinitSetting[0]));
}

int ES7210_3AmicInit(void)
{
    return _ES7210_Init(u8Es7210_3AmicInitSetting,
            sizeof(u8Es7210_3AmicInitSetting) / sizeof(u8Es7210_3AmicInitSetting[0]));
}

int ES7210_3AmicDeInit(void)
{
    return _ES7210_Deinit(u8Es7210_3AmicDeinitSetting,
            sizeof(u8Es7210_3AmicDeinitSetting) / sizeof(u8Es7210NormalDeinitSetting[0]));
}
