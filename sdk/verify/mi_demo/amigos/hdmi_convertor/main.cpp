#include <stdio.h>
#include "vif.h"
#include "vpe.h"
#include "ai.h"
#include "sys.h"
#include "rtsp.h"
#include "venc.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

//#include "es8156.h"
#include "mi_sensor.h"
#include "tem.h"


#define I2C_ADAPTER_STR     ("/dev/i2c-1")
#define ES8156_CHIP_ADDR   (0x08)

typedef enum
{
    EN_CUST_CMD_GET_STATE,
    EN_CUST_CMD_GET_AUDIO_INFO,
    EN_CUST_CMD_MAX
}EN_TC358743_SENSOR_STATE;
typedef enum
{
    E_S0_STATUS_POWEROFF,
    E_RS1_STATUS_SETEDID,
    E_RS2_STATUS_SETHPDO,
    E_RS3_STATUS_ENMIPIOUT,
    E_RS4_STATUS_NOSIGNAL,
    E_RS5_STATUS_NOCONNECTION,
}Tcxbg_state_e;
typedef struct
{
    unsigned char bPcmData;
    unsigned char ucharBitWidth;
    unsigned char ucharChannels;
    unsigned int uintSampleRate;
}Tcxbg_audio_info_t;
typedef struct
{
    std::vector<Sys *> * pVectVideoPipeLine;
    std::vector<Sys *> * pVectAudioPipeLine;
    Tcxbg_state_e enCurState;
}stMonitorDataPackage_t;

typedef enum
{
    E_START_MONITOR,
    E_EXIT_MONITOR
}EN_TC_MonitorCmd_e;

static const unsigned char u8Es8156InitSetting[][2] = {
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
    {0x11, 0x31},
};

static const unsigned char u8Es8156DeinitSetting[][2] = {
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
    packets.msgs = messages;

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

static int ES8156_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_SelectWordLength(unsigned char ucharWordLength)
{
    int s32Ret;
    unsigned char val;

    s32Ret = ES8156_ReadByte(0x11, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 reg 0x11 :%x.\n", val);
    }
    else
    {
        return s32Ret;
    }
    switch (ucharWordLength)
    {
        case 16:
        {
            val = (0x3 << 4) | (val & 0xF);
        }
        break;
        case 18:
        {
            val = (0x2 << 4) | (val & 0xF);
        }
        break;
        case 20:
        {
            val = (0x1 << 4) | (val & 0xF);
        }
        break;
        case 24:
        {
            val = val & 0xF;
        }
        break;
        case 32:
        {
            val = (0x4 << 4) | (val & 0xF);
        }
        break;
        default:
            val = (0x3 << 4) | (val & 0xF);
            break;
    }
    s32Ret = ES8156_WriteByte(0x11, val);
    if (0 != s32Ret)
    {
        return s32Ret;
    }
    printf("Write val 0x%x\n", val);

    return 0;

}

static int ES8156_Init(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    I2C_Init(10, 5);
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

static int ES8156_Deinit(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

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
    I2C_Deinit();

	printf("Deinit ES8156 success.\n");
    return 0;
}

void Sys::Implement(std::string &strKey)
{
    unsigned int intId = 0;

    //printf("Connect key str %s\n", strKey.c_str());
    intId = Sys::FindBlockId(strKey);
    if (intId == (unsigned int)-1)
    {
        printf("Can't find key str %s\n", strKey.c_str());
        return;
    }
    if (!Sys::FindBlock(strKey))
    {
        switch (intId)
        {
            case E_SYS_MOD_RTSP:
            {
                SysChild<Rtsp> Rtsp(strKey);
            }
            break;
#if INTERFACE_VENC
            case E_SYS_MOD_VENC:
            {
                SysChild<Venc> Venc(strKey);
            }
            break;
#endif
#if INTERFACE_VPE
            case E_SYS_MOD_VPE:
            {
                SysChild<Vpe> Vpe(strKey);
            }
            break;
#endif
#if INTERFACE_VIF
            case E_SYS_MOD_VIF:
            {
                SysChild<Vif> Vif(strKey);
            }
            break;
#endif
#if INTERFACE_AI
            case E_SYS_MOD_AI:
            {
                SysChild<Ai> Ai(strKey);
            }
            break;
#endif
            default:
                return;
        }
        GetInstance(strKey)->BuildModTree();
    }

    return;
}
void *Tc358743xbgDoCmd(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stData)
{
    EN_TC_MonitorCmd_e *penCmd;
    stMonitorDataPackage_t *pstPackage;

    ASSERT(stData.u32UserDataSize == sizeof(EN_TC_MonitorCmd_e));
    ASSERT(sizeof(stMonitorDataPackage_t) == stBuf.u32TemBufferSize);

    pstPackage = (stMonitorDataPackage_t *)stBuf.pTemBuffer;
    penCmd = (EN_TC_MonitorCmd_e *)stData.pUserData;
    switch (*penCmd)
    {
        case E_START_MONITOR:
        {
            printf("Nothing to do !\n");
        }
        break;
        case E_EXIT_MONITOR:
        {
            if (pstPackage->enCurState == E_RS3_STATUS_ENMIPIOUT)
            {
                ES8156_Deinit();
                printf("Signal lock need release!\n");
                Sys::Extract(*pstPackage->pVectAudioPipeLine);
                printf("Release audio done !\n");
                Sys::Extract(*pstPackage->pVectVideoPipeLine);
                printf("Release video done !\n");
            }
        }
        break;
        default:
            ASSERT(0);
    }

    return NULL;
}

static void * Tc358743xbgSensorMonitor(ST_TEM_BUFFER stBuf)
{
    Tcxbg_state_e enTcState;
    stMonitorDataPackage_t *pstPackage;
    Tcxbg_audio_info_t stAudioInfo;

    ASSERT(sizeof(stMonitorDataPackage_t) == stBuf.u32TemBufferSize);
    MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_STATE, sizeof(Tcxbg_state_e), (void *)&enTcState, E_MI_SNR_CUSTDATA_TO_USER);
    pstPackage = (stMonitorDataPackage_t *)stBuf.pTemBuffer;
    if (pstPackage->enCurState != E_RS3_STATUS_ENMIPIOUT && enTcState == E_RS3_STATUS_ENMIPIOUT)
    {
        MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_AUDIO_INFO, sizeof(Tcxbg_audio_info_t), (void *)&stAudioInfo, E_MI_SNR_CUSTDATA_TO_USER);
        printf("Signal lock %d pcm %d audio sample rate %d audio bit width %d channels %d\n", enTcState, stAudioInfo.bPcmData, stAudioInfo.uintSampleRate, stAudioInfo.ucharBitWidth, stAudioInfo.ucharChannels);
        Sys::Insert(*pstPackage->pVectVideoPipeLine);
        printf("Insert audio done !\n");
        Sys::Insert(*pstPackage->pVectAudioPipeLine);
        printf("Insert video done !\n");
        ES8156_Init();
        ES8156_SelectWordLength(stAudioInfo.ucharBitWidth);

    }
    else if (pstPackage->enCurState == E_RS3_STATUS_ENMIPIOUT && enTcState != E_RS3_STATUS_ENMIPIOUT)
    {
        ES8156_Deinit();
        printf("Signal unlock %d\n", enTcState);
        Sys::Extract(*pstPackage->pVectAudioPipeLine);
        printf("Release audio done !\n");
        Sys::Extract(*pstPackage->pVectVideoPipeLine);
        printf("Release video done !\n");

    }
    pstPackage->enCurState = enTcState;
    return NULL;
}
static void tc358743xbgInit(std::vector<Sys *> *pVectVideoPipeLine, std::vector<Sys *> *pVectAudioPipeLine)
{
    ST_TEM_ATTR stTemAttr;
    stMonitorDataPackage_t stTmpPackage;
    ST_TEM_USER_DATA stData;
    EN_TC_MonitorCmd_e enCmd;

    memset(&stTmpPackage, 0, sizeof(stMonitorDataPackage_t));
    stTmpPackage.enCurState = E_S0_STATUS_POWEROFF;
    stTmpPackage.pVectVideoPipeLine = pVectVideoPipeLine;
    stTmpPackage.pVectAudioPipeLine = pVectAudioPipeLine;
    MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)0, FALSE);
    MI_SNR_SetRes((MI_SNR_PAD_ID_e)0, 0);
    MI_SNR_Enable((MI_SNR_PAD_ID_e)0);    
    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = Tc358743xbgDoCmd;
    stTemAttr.fpThreadWaitTimeOut = Tc358743xbgSensorMonitor;
    stTemAttr.u32ThreadTimeoutMs = 500;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = &stTmpPackage;
    stTemAttr.stTemBuf.u32TemBufferSize = sizeof(stMonitorDataPackage_t);
    TemOpen("tc358743xbg_monitor", stTemAttr);
    enCmd = E_START_MONITOR;
    stData.pUserData = (void *)&enCmd;
    stData.u32UserDataSize = sizeof(EN_TC_MonitorCmd_e);
    TemSend("tc358743xbg_monitor", stData);
    TemStartMonitor("tc358743xbg_monitor");
}
static void tc358743xbgDeinit()
{
    ST_TEM_USER_DATA stData;
    EN_TC_MonitorCmd_e enCmd;

    TemStop("tc358743xbg_monitor");
    enCmd = E_EXIT_MONITOR;
    stData.pUserData = (void *)&enCmd;
    stData.u32UserDataSize = sizeof(EN_TC_MonitorCmd_e);
    TemSend("tc358743xbg_monitor", stData);
    TemClose("tc358743xbg_monitor");
}
int main(int argc, char **argv)
{
    std::map<std::string, unsigned int> mapModId;
    std::map<std::string, Sys *> maskMap;
    std::vector<Sys *> vectVideoPipeLine;
    std::vector<Sys *> vectAudioPipeLine;
    std::string objName;
    Vif *VifObj = NULL;
    Vpe *VpeObj = NULL;
    Ai *AiObj = NULL;
    Venc *VencObj = NULL;

    char getC = 0;

    if (argc != 2)
    {
        printf("Usage: ./%s xxx_ini_path\n", argv[0]);

        return -1;
    }
    mapModId["RTSP"] = E_SYS_MOD_RTSP;
    mapModId["VENC"] = E_SYS_MOD_VENC;
    mapModId["VPE"] = E_SYS_MOD_VPE;
    mapModId["VIF"] = E_SYS_MOD_VIF;
    mapModId["AI"] = E_SYS_MOD_AI;
    Sys::CreateObj(argv[1], mapModId);

    objName = "VIF_CH0_DEV0";
    VifObj = dynamic_cast<Vif *>(Sys::GetInstance(objName));
    if (!VifObj)
    {
        printf("Obj error!\n");        
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VifObj);
    maskMap[objName] = VifObj;

    objName = "VPE_CH0_DEV0";
    VpeObj = dynamic_cast<Vpe *>(Sys::GetInstance(objName));   
    if (!VpeObj)
    {
        printf("Obj error!\n");
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VpeObj);
    maskMap[objName] = VpeObj;

    objName = "VENC_CH0_DEV0";
    VencObj = dynamic_cast<Venc *>(Sys::GetInstance(objName));   
    if (!VencObj)
    {
        printf("Obj error!\n");
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VencObj);
    maskMap[objName] = VencObj;


    objName = "AI_CH0_DEV0";
    AiObj = dynamic_cast<Ai *>(Sys::GetInstance(objName));   
    if (!AiObj)
    {
        printf("Obj error!\n");
        Sys::DestroyObj();
        return -1;
    }
    
    vectAudioPipeLine.push_back(AiObj);
    maskMap[objName] = AiObj;
    Sys::Begin(maskMap);
    tc358743xbgInit(&vectVideoPipeLine, &vectAudioPipeLine);
    do
    {
        printf("Press 'q' to exit!\n");
        getC = getchar();
    }while (getC != 'q');
    tc358743xbgDeinit();
    Sys::End(maskMap);
    Sys::DestroyObj();

    return 0;
}
