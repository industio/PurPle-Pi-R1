#ifndef __I2C__
#define __I2C__
int I2C_Init(unsigned int timeout, unsigned int retryTime);
int I2C_Deinit(void);
int I2C_GetFd(void);
int I2C_WriteByte(unsigned char reg, unsigned char val, unsigned char i2cAddr);
int I2C_ReadByte(unsigned char reg, unsigned char *val, unsigned char i2cAddr);
#endif
