#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define I2C_ADAPTER_STR     ("/dev/i2c-1")

static int s8I2cAdapterFd = -1;
static int bI2cInit = 0;

int I2C_Init(unsigned int timeout, unsigned int retryTime)
{
    int s32Ret;
    if (0 == bI2cInit)
    {
        s8I2cAdapterFd = open((const char *)I2C_ADAPTER_STR, O_RDWR);
        if (-1 == s8I2cAdapterFd)
        {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_TIMEOUT, timeout);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c time out param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_RETRIES, retryTime);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c retry time param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        bI2cInit = 1;
    }
    return 0;
}

int I2C_Deinit(void)
{
    if (1 == bI2cInit)
    {
        close(s8I2cAdapterFd);
        bI2cInit = 0;
    }
    return 0;
}

int I2C_GetFd(void)
{
    return s8I2cAdapterFd;
}

int I2C_WriteByte(unsigned char reg, unsigned char val, unsigned char i2cAddr)
{
    int s32Ret;
    unsigned char data[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;

    if (-1 == I2C_GetFd())
    {
        printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    // send one message
    packets.nmsgs = 1;
    packets.msgs = &messages;

    // fill message
    messages.addr = i2cAddr;   // codec reg addr
    messages.flags = 0;                 // read/write flag, 0--write, 1--read
    messages.len = 2;                   // size
    messages.buf = data;                // data addr

    // fill data
    data[0] = reg;
    data[1] = val;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to write byte to ES7210: %s.\n", strerror(errno));
        return -1;
    }

    return 0;
}

int I2C_ReadByte(unsigned char reg, unsigned char *val, unsigned char i2cAddr)
{
    int s32Ret;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    unsigned char tmpReg, tmpVal;

    if (-1 == I2C_GetFd())
    {
        printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    if (NULL == val)
    {
        printf("val param is NULL.\n");
        return -1;
    }

    tmpReg = reg;
    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    packets.nmsgs = 2;
    packets.msgs = &messages;

    messages[0].addr = i2cAddr;
    messages[0].flags = 0;
    messages[0].len = 1;
    messages[0].buf = &tmpReg;

    tmpVal = 0;
    messages[1].addr = i2cAddr;
    messages[1].flags = 1;
    messages[1].len = 1;
    messages[1].buf = &tmpVal;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to read byte from ES7210: %s.\n", strerror(errno));
        return -1;
    }

    *val = tmpVal;
    return 0;
}
