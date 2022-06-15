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

#include <stdio.h>
#include <string.h>

#include "mi_common.h"
#include "mi_sys.h"
#if INTERFACE_VENC
#include "mi_venc.h"
#endif
#if INTERFACE_VDEC
#include "mi_vdec.h"
#endif
#if INTERFACE_AI
#include "mi_ai.h"
#endif

#include "sys.h"
#include "venc.h"


std::map<std::string, Sys *> Sys::connectMap;
std::map<std::string, unsigned int> Sys::connectIdMap;
std::vector<Sys *> Sys::connectOrder;
dictionary * Sys::m_pstDict = NULL;


typedef struct stSys_BindInfo_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    MI_SYS_BindType_e eBindType;
    MI_U32 u32BindParam;
} stSys_BindInfo_T;
static MI_S32 Sys_Init(void)
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;

    STCHECKRESULT(MI_SYS_Init());

    memset(&stVersion, 0x0, sizeof(MI_SYS_Version_t));
    STCHECKRESULT(MI_SYS_GetVersion(&stVersion));
    printf("u8Version:%s\n", stVersion.u8Version);

    STCHECKRESULT(MI_SYS_GetCurPts(&u64Pts));
    printf("u64Pts:0x%llx\n", u64Pts);

    u64Pts = 0xF1237890F1237890;
    STCHECKRESULT(MI_SYS_InitPtsBase(u64Pts));

    u64Pts = 0xE1237890E1237890;
    STCHECKRESULT(MI_SYS_SyncPts(u64Pts));

    return MI_SUCCESS;
}

static MI_S32 Sys_Exit(void)
{
    STCHECKRESULT(MI_SYS_Exit());

    return MI_SUCCESS;
}


int Sys::GetIniInt(std::string section, std::string key)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        printf("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    return iniparser_getint(m_pstDict, strTmp.c_str(), -1);
}
unsigned int Sys::GetIniUnsignedInt(std::string section, std::string key)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        printf("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    return iniparser_getunsignedint(m_pstDict, strTmp.c_str(), -1);
}
char* Sys::GetIniString(std::string section, std::string key)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        printf("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    return iniparser_getstring(m_pstDict, strTmp.c_str(), NULL);
}
void Sys::InitSys(std::string strIniPath, std::map<std::string, unsigned int> &mapModId)
{
    std::map<std::string, Sys *> maskMap;

    CreateObj(strIniPath, mapModId);
    maskMap.clear();
    Begin(maskMap);
}
void Sys::DeinitSys()
{
    std::map<std::string, Sys *> maskMap;

    maskMap.clear();
    End(maskMap);
    DestroyObj();
}
void Sys::CreateObj(std::string strIniPath, std::map<std::string, unsigned int> &mapModId)
{
    unsigned int i = 0;
    Sys *pClass = NULL;

    if (!m_pstDict)
    {
        m_pstDict = iniparser_load(strIniPath.c_str());
    }
    connectIdMap = mapModId;
    CreateConnection();
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        pClass->LoadDb();
    }
    Sys_Init();
}
void Sys::DestroyObj()
{
    Sys_Exit();
    DestroyConnection();
    connectIdMap.clear();
    if (m_pstDict)
    {
        iniparser_freedict(m_pstDict);
    }
}
void Sys::Begin(std::map<std::string, Sys *> &maskMap)
{
    unsigned int i = 0;
    Sys *pClass = NULL;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;

    //init
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            pClass->bExtract = 1;
            continue;
        }
        pClass->Init();
    }

    //bind
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        for (itMapIn = pClass->mapModInputInfo.begin(); itMapIn != pClass->mapModInputInfo.end(); ++itMapIn)
        {
            if (maskMap.find(itMapIn->second.stPrev.modKeyString) != maskMap.end())
            {
                continue;
            }
            pClass->BindBlock(itMapIn->second);
        }
    }

    //start
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Start();
    }

}
void Sys::End(std::map<std::string, Sys *> &maskMap)
{
    Sys *pClass = NULL;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    unsigned int i = 0;

    //stop
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Stop();
    }

    //unbind
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        for (itMapIn = pClass->mapModInputInfo.begin(); itMapIn != pClass->mapModInputInfo.end(); ++itMapIn)
        {
            if (maskMap.find(itMapIn->second.stPrev.modKeyString) != maskMap.end())
            {
                continue;
            }
            pClass->UnBindBlock(itMapIn->second);
        }
    }

    //deinit
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Deinit();
    }

}
void Sys::Extract(std::vector<Sys *> &objVect)
{
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;
    std::vector<stModIoInfo_t>::iterator itOutMod;
    Sys *pNextClass = NULL;
    Sys *Object = NULL;
    unsigned int i = 0;

    for (i = objVect.size(); i != 0; i--)
    {
        Object = objVect[i - 1];
        Object->Stop();
    }
    for (i = objVect.size(); i != 0; i--)
    {

        Object = objVect[i - 1];
        if (i == objVect.size())
        {
            for (itMapOut = Object->mapModOutputInfo.begin(); itMapOut != Object->mapModOutputInfo.end(); ++itMapOut)
            {
                for (itOutMod = itMapOut->second.vectNext.begin(); itOutMod !=  itMapOut->second.vectNext.end(); ++itOutMod)
                {
                    pNextClass = GetInstance(itOutMod->modKeyString);
                    pNextClass->UnBindBlock(pNextClass->mapModInputInfo[itOutMod->portId]);
                }
            }
        }
        for (itMapIn = Object->mapModInputInfo.begin(); itMapIn != Object->mapModInputInfo.end(); ++itMapIn)
        {
            Object->UnBindBlock(itMapIn->second);
        }
    }
    for (i = objVect.size(); i != 0; i--)
    {
        Object = objVect[i - 1];
        Object->Deinit();
        Object->bExtract = 1;
    }
}
void Sys::Insert(std::vector<Sys *> &objVect)
{
    std::vector<Sys *>::iterator itObjVect;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;
    std::vector<stModIoInfo_t>::iterator itOutMod;
    Sys *pNextClass = NULL;
    Sys *Object = NULL;
    unsigned int uintSz = 0;

    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        Object->Init();
    }
    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        for (itMapIn = Object->mapModInputInfo.begin(); itMapIn != Object->mapModInputInfo.end(); ++itMapIn)
        {
            Object->BindBlock(itMapIn->second);
        }
        if (++uintSz == objVect.size())
        {
            for (itMapOut = Object->mapModOutputInfo.begin(); itMapOut != Object->mapModOutputInfo.end(); ++itMapOut)
            {
                for (itOutMod = itMapOut->second.vectNext.begin(); itOutMod !=  itMapOut->second.vectNext.end(); ++itOutMod)
                {
                    pNextClass = GetInstance(itOutMod->modKeyString);
                    pNextClass->BindBlock(pNextClass->mapModInputInfo[itOutMod->portId]);
                }
            }
        }
    }
    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        Object->Start();
        Object->bExtract = 0;
    }
}
void Sys::CreateConnection()
{
    std::map<unsigned int, std::string>::iterator it;
    std::map<unsigned int, std::string> mapRootKeyStr;
    int rootCnt;
    char root[30];
    std::string rootName;
    int i = 0;

    printf("%d\n",iniparser_getint(m_pstDict, "ROOT:COUNT", -1));
    rootCnt = iniparser_getint(m_pstDict, "ROOT:COUNT", -1);
    for(i = 0; i < rootCnt; i++)
    {
        sprintf(root,"ROOT:NAME_%d",i);
        rootName = iniparser_getstring(m_pstDict, root, NULL);
        printf("root %s\n",rootName.c_str());
        mapRootKeyStr[i] = rootName;
    }
    for (it = mapRootKeyStr.begin(); it != mapRootKeyStr.end(); ++it)
    {
        Implement(it->second);
    }
}
void Sys::DestroyConnection(void)
{
    Sys *pClass = NULL;
    unsigned int i = 0;

    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        pClass->mapModInputInfo.clear();
        pClass->mapModOutputInfo.clear();
        delete (pClass);
        connectOrder[i] = NULL;
    }
    connectMap.clear();
    connectOrder.clear();
}
void Sys::SetCurInfo(std::string &strKey)
{
    unsigned int inCnt = 0;
    unsigned int outCnt = 0;
    unsigned int count = 0;
    int tmpPos = 0;
    char tmpStr[20];
    char *pRes = NULL;
    unsigned int i = 0;
    std::string strTempString;
    std::string strTempPrevPort;
    stModInputInfo_t stInputInfo;
    stModOutputInfo_t stOutputInfo;

    inCnt = GetIniUnsignedInt(strKey, "IN_CNT");
    outCnt = GetIniUnsignedInt(strKey, "OUT_CNT");
    stModDesc.modKeyString = strKey;
    stModDesc.modId = FindBlockId(strKey);
    stModDesc.chnId = GetIniUnsignedInt(strKey, "CHN");
    stModDesc.devId = GetIniUnsignedInt(strKey, "DEV");
    
    if (inCnt)
    {
        for (i = 0; i < MAX_INPUT_CNT; i++)
        {
            memset(tmpStr, 0, 20);
            sprintf(tmpStr, "IN_%d", i);
            pRes = GetIniString(strKey, tmpStr);
            if(pRes != NULL)
            {
                stInputInfo.curIoKeyString = pRes;
                //printf("inKeyStr %s\n", stInputInfo.curIoKeyString.c_str());
                strTempString = GetIniString(pRes, "PREV");
                tmpPos = strTempString.find_first_of(':');
                strTempPrevPort = strTempString;
                strTempPrevPort.erase(0, tmpPos + 1);
                stInputInfo.stPrev.modKeyString = strTempString.erase(tmpPos, strTempString.size() - tmpPos);

                strTempString.erase();
                strTempString = GetIniString(stInputInfo.stPrev.modKeyString, strTempPrevPort.c_str());
                stInputInfo.stPrev.portId = GetIniUnsignedInt(strTempString, "PORT");
                stInputInfo.stPrev.frmRate = GetIniUnsignedInt(strTempString, "FPS");
                stInputInfo.curPortId = GetIniUnsignedInt(pRes, "PORT");
                stInputInfo.curFrmRate = GetIniUnsignedInt(pRes, "FPS");
                if (stInputInfo.curIoKeyString.find("VDISP") != std::string::npos)
                {
                    int curChnId =  GetIniUnsignedInt(pRes, "CHN");
                    mapModInputInfo[curChnId] = stInputInfo;
                }
                else
                {
                    mapModInputInfo[stInputInfo.curPortId] = stInputInfo;
                }
                count++;
            }
            if(count == inCnt)
            {
                break;
            }
        }
    }
    if (outCnt)
    {
        count = 0;
        for (i = 0; i < MAX_OUTPUT_CNT; i++)
        {
            memset(tmpStr, 0, 20);
            sprintf(tmpStr, "OUT_%d", i);
            pRes = GetIniString(strKey, tmpStr);
            if(pRes != NULL)
            {
                stOutputInfo.curIoKeyString = pRes;
                stOutputInfo.curPortId = GetIniUnsignedInt(pRes, "PORT");
                stOutputInfo.curFrmRate = GetIniUnsignedInt(pRes, "FPS");
                mapModOutputInfo[stOutputInfo.curPortId] = stOutputInfo;
                count++;
            }
            if(count == outCnt)
            {
                break;
            }
        }
    }
    connectMap[stModDesc.modKeyString] = this;
    connectOrder.push_back(this);

    return;
}
void Sys::BuildModTree()
{
    Sys *pPreClass;
    stModIoInfo_t stIoInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapInput;

    for (itMapInput = mapModInputInfo.begin(); itMapInput != mapModInputInfo.end(); ++itMapInput)
    {
        Implement(itMapInput->second.stPrev.modKeyString);
        pPreClass =  GetInstance(itMapInput->second.stPrev.modKeyString);
        if (pPreClass)
        {
            stIoInfo.modKeyString = stModDesc.modKeyString;
            stIoInfo.portId = itMapInput->second.curPortId;
            stIoInfo.frmRate = itMapInput->second.curFrmRate;
            pPreClass->mapModOutputInfo[itMapInput->second.stPrev.portId].vectNext.push_back(stIoInfo);
        }
    }
}
void Sys::Start()
{
}
void Sys::Stop()
{
}
void Sys::BindBlock(stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc;
    stSys_BindInfo_T stBindInfo;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);

    if(stPreDesc.modId > E_SYS_MOD_EXT)
    {
        CreateReceiver(stIn.curPortId, DataReceiver, NULL, NULL, this);
        StartReceiver(stIn.curPortId);
    }
    else if (stPreDesc.modId < E_SYS_MOD_INT_MAX)
    {
        //printf("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
        //printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
        memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
        stBindInfo.eBindType = (MI_SYS_BindType_e)GetIniInt(stIn.curIoKeyString, "BIND_TYPE");
        stBindInfo.u32BindParam = GetIniInt(stIn.curIoKeyString, "BIND_PARAM");
        stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
        stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
        stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
        stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
        stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
        stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
        stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;

        if(stPreDesc.modId == E_SYS_MOD_VDEC || stPreDesc.modId == E_SYS_MOD_VDISP)
        {
            if(stModDesc.modId == E_SYS_MOD_VDISP)
            {
                stBindInfo.stDstChnPort.u32ChnId = GetIniInt(stIn.curIoKeyString, "CHN");
            }
            MI_SYS_BindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate);
        }
        else
        {
            MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
        }
    }
}
void Sys::UnBindBlock(stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc;
    stSys_BindInfo_T stBindInfo;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);

    if(stPreDesc.modId > E_SYS_MOD_EXT)
    {
        StopReceiver(stIn.curPortId);
        DestroyReceiver(stIn.curPortId);
    }
    else if (stPreDesc.modId < E_SYS_MOD_INT_MAX)
    {
        //printf("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
        //printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
        memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
        stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
        stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
        stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
        stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
        stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
        stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
        stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;

         if(stPreDesc.modId == E_SYS_MOD_VDEC)
         {
            stBindInfo.stDstChnPort.u32ChnId = GetIniInt(stIn.curIoKeyString, "CHN");
         }

        MI_SYS_UnBindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort);
    }
}


int Sys::CreateSender(unsigned int outPortId)
{
    ST_TEM_ATTR stTemAttr;
    stReceiverDesc_t stDest;

    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = SenderState;
    stTemAttr.fpThreadWaitTimeOut = SenderMonitor;
    stTemAttr.u32ThreadTimeoutMs = 10;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = (void *)&(mapRecevier[outPortId]);
    stTemAttr.stTemBuf.u32TemBufferSize = 0;
    TemOpen(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stTemAttr);

    return 0;
}
int Sys::StartSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc)
{
    ST_TEM_USER_DATA stUsrData;
    stSenderState_t stState;
    stState.eState = E_SENDER_STATE_START;
    stState.pData = (void *)&stRecvPortDesc;

    stUsrData.pUserData = &stState;
    stUsrData.u32UserDataSize = sizeof(stSenderState_t);
    TemSend(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stUsrData);
    if (mapRecevier[outPortId].uintRefsCnt == 0)
    {
        TemStartMonitor(mapModOutputInfo[outPortId].curIoKeyString.c_str());
    }
    mapRecevier[outPortId].uintRefsCnt++;

    return 0;
}
int Sys::StopSender(unsigned int outPortId, stReceiverPortDesc_t &stRecvPortDesc)
{
    ST_TEM_USER_DATA stUsrData;
    stSenderState_t stState;
    stState.eState = E_SENDER_STATE_STOP;
    stState.pData = (void *)&stRecvPortDesc;

    if (mapRecevier[outPortId].uintRefsCnt)
        mapRecevier[outPortId].uintRefsCnt--;
    if (mapRecevier[outPortId].uintRefsCnt == 0)
    {
        TemStop(mapModOutputInfo[outPortId].curIoKeyString.c_str());
    }
    stUsrData.pUserData = &stState;
    stUsrData.u32UserDataSize = sizeof(stSenderState_t);
    TemSend(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stUsrData);

    return 0;
}
int Sys::DestroySender(unsigned int outPortId)
{
    TemClose(mapModOutputInfo[outPortId].curIoKeyString.c_str());

    return 0;
}
int Sys::State(unsigned int outPortId, E_SENDER_STATE eState, stReceiverPortDesc_t &stRecPortDesc)
{
    pthread_mutex_lock(&mapRecevier[outPortId].stDeliveryMutex);
    if (stRecPortDesc.bStart)
    {
        switch (eState)
        {
            case E_SENDER_STATE_START:
            {
                if (stRecPortDesc.fpStateStart)
                    stRecPortDesc.fpStateStart(stRecPortDesc.pUsrData);
            }
            break;
            case E_SENDER_STATE_STOP:
            {
                if (stRecPortDesc.fpStateStop)
                    stRecPortDesc.fpStateStop(stRecPortDesc.pUsrData);
            }
            break;
            default:
                assert(0);
        }
    }
    pthread_mutex_unlock(&mapRecevier[outPortId].stDeliveryMutex);

    return 0;
}
int Sys::Send(unsigned int outPortId, void *pData, unsigned int intDataSize)
{
    std::map<std::string, stReceiverPortDesc_t>::iterator it;
    std::map<std::string, stReceiverPortDesc_t> *pMap = &mapRecevier[outPortId].mapPortDesc;

    pthread_mutex_lock(&mapRecevier[outPortId].stDeliveryMutex);
    for (it = pMap->begin(); it != pMap->end(); ++it)
    {
        if (it->second.bStart)
        {
            it->second.fpRec(pData, intDataSize, it->second.pUsrData, it->second.portId);
        }
    }
    pthread_mutex_unlock(&mapRecevier[outPortId].stDeliveryMutex);

    return 0;
}
void *Sys::SenderState(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stUsrData)
{
    stSenderState_t stSenderState;
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    Sys *pClass = pReceiver->pSysClass;

    assert(stUsrData.pUserData);
    assert(stUsrData.u32UserDataSize == sizeof(stSenderState_t));
    stSenderState = *((stSenderState_t *)stUsrData.pUserData);
    assert(stSenderState.pData);
    pClass->State(pReceiver->uintPort, stSenderState.eState, *((stReceiverPortDesc_t *)stSenderState.pData));

    return NULL;
}

int Sys::CreateReceiver(unsigned int inPortId, DeliveryRecFp funcRecFp, DeliveryState funcStart, DeliveryState funcStop, void *pUsrData)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    stReceiverDesc_t stReceiverDesc;
    stReceiverPortDesc_t stReceiverPortDesc;

    if (!funcRecFp)
    {
        printf("funcRecFp is null!\n");

        return -1;
    }
    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        printf("Can not find input port %d\n", inPortId);
        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        printf("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    stReceiverPortDesc.bStart = FALSE;
    stReceiverPortDesc.fpRec = funcRecFp;
    stReceiverPortDesc.fpStateStart = funcStart;
    stReceiverPortDesc.fpStateStop = funcStop;
    stReceiverPortDesc.pUsrData = pUsrData;
    stReceiverPortDesc.portId = inPortId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) == pPrevClass->mapRecevier.end())
    {
        stReceiverDesc.mapPortDesc[mapModInputInfo[inPortId].curIoKeyString] = stReceiverPortDesc;
        stReceiverDesc.pSysClass = pPrevClass;
        stReceiverDesc.uintPort = intPrevOutPort;
        stReceiverDesc.stDeliveryMutex = PTHREAD_MUTEX_INITIALIZER;
        stReceiverDesc.uintRefsCnt = 0;
        pPrevClass->mapRecevier[intPrevOutPort] = stReceiverDesc;
        pPrevClass->CreateSender(intPrevOutPort);
    }
    else
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString] = stReceiverPortDesc;
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
    }

    return 0;
}
int Sys::StartReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        printf("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        printf("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString].bStart = TRUE;
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->StartSender(intPrevOutPort, pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString]);
    }
    else
    {
        printf("Receiver did not create. inpot id %d current %s prev %s\n", inPortId, mapModInputInfo[inPortId].curIoKeyString.c_str(), mapModInputInfo[inPortId].stPrev.modKeyString.c_str());

        return -1;
    }

    return 0;
}
int Sys::StopReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    stReceiverDesc_t stReceiverDesc;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        printf("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        printf("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        pPrevClass->StopSender(intPrevOutPort, pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString]);
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString].bStart = FALSE;
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
    }
    else
    {
        printf("Receiver did not create. inpot id %d current %s prev %s\n", inPortId, mapModInputInfo[inPortId].curIoKeyString.c_str(), mapModInputInfo[inPortId].stPrev.modKeyString.c_str());

        return -1;
    }

    return 0;
}
int Sys::DestroyReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    std::map<unsigned int, stReceiverDesc_t>::iterator it;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        printf("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        printf("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        if (pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.find(mapModInputInfo[inPortId].curIoKeyString) != pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.end())
        {
            pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.erase(mapModInputInfo[inPortId].curIoKeyString);
        }
        if (pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.size() == 0)
        {
            pPrevClass->DestroySender(intPrevOutPort);
            pPrevClass->mapRecevier.erase(intPrevOutPort);
        }
    }

    return 0;
}

void * Sys::SenderMonitor(ST_TEM_BUFFER stBuf)
{
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_S32 s32Fd;
    MI_S32 s32Ret;
    fd_set read_fds;
    struct timeval tv;
#if INTERFACE_VENC
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack[16];
    MI_VENC_ChnStat_t stStat;
#endif
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    Sys *pClass = pReceiver->pSysClass;
    stStreamInfo_t stStreamInfo;

    if (pClass->bExtract)
        return NULL;
    memset(&stStreamInfo, 0, sizeof(stStreamInfo));
    stChnOutputPort.eModId = (MI_ModuleId_e)pClass->stModDesc.modId;
    stChnOutputPort.u32DevId = (MI_U32)pClass->stModDesc.devId;
    stChnOutputPort.u32ChnId = (MI_U32)pClass->stModDesc.chnId;
    stChnOutputPort.u32PortId = (MI_U32)pReceiver->uintPort;
    switch (pClass->stModDesc.modId)
    {
        case E_SYS_MOD_VPE:
        case E_SYS_MOD_DIVP:
        {
            s32Ret = MI_SYS_GetFd(&stChnOutputPort, &s32Fd);
            if (s32Ret < 0)
            {
                return NULL;
            }

            FD_ZERO(&read_fds);
            FD_SET(s32Fd, &read_fds);
            tv.tv_sec = 0;
            tv.tv_usec = 10 * 1000;
            s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
            if (s32Ret < 0)
            {
                MI_SYS_CloseFd(s32Fd);

                return NULL;
            }
            else if (0 == s32Ret)
            {
                MI_SYS_CloseFd(s32Fd);

                return NULL;
            }
            if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stChnOutputPort , &stBufInfo,&hHandle))
            {
                stStreamInfo.eStreamType = (E_STREAM_TYPE)stBufInfo.stFrameData.ePixelFormat;
                stStreamInfo.streamWidth = stBufInfo.stFrameData.u16Width;
                stStreamInfo.streamHeight = stBufInfo.stFrameData.u16Height;
                if(stStreamInfo.eStreamType == E_STREAM_YUV420)
                {
                    stStreamInfo.stYuvInfo.pYdataAddr = (char*)stBufInfo.stFrameData.pVirAddr[0];
                    stStreamInfo.stYuvInfo.pUvDataAddr = (char*)stBufInfo.stFrameData.pVirAddr[1];
                }
                else if(stStreamInfo.eStreamType == E_STREAM_YUV422)
                {
                    stStreamInfo.stYuvInfo.pYuvDataAddr = (char*)stBufInfo.stFrameData.pVirAddr[0];
                }
                pClass->Send(pReceiver->uintPort, &stStreamInfo, sizeof(stStreamInfo));
                MI_SYS_ChnOutputPortPutBuf(hHandle);
            }
            MI_SYS_CloseFd(s32Fd);
        }
        break;
#if INTERFACE_AI
        case E_SYS_MOD_AI:
        {
            MI_AUDIO_Frame_t stFrm;
            MI_AUDIO_AecFrame_t stAecFrm;
            stEsPackage_t stEsPacket;

            memset(&stFrm, 0, sizeof(MI_AUDIO_Frame_t));
            memset(&stAecFrm, 0, sizeof(MI_AUDIO_AecFrame_t));
            if (MI_SUCCESS == MI_AI_GetFrame((MI_AUDIO_DEV)pClass->stModDesc.devId, (MI_AI_CHN)pClass->stModDesc.chnId, &stFrm, &stAecFrm, 40))
            {
                stStreamInfo.eStreamType = E_STREAM_PCM;
                stStreamInfo.stCodecInfo.uintPackCnt = 1;
                stEsPacket.pData = (char *)stFrm.apSrcPcmVirAddr[0];
                stEsPacket.uintDataSize = stFrm.u32SrcPcmLen;
                stStreamInfo.stCodecInfo.pDataAddr = &stEsPacket;
                stStreamInfo.stCodecInfo.uintPackCnt = 1;
                pClass->Send(pReceiver->uintPort, &stStreamInfo, sizeof(stStreamInfo));
                MI_AI_ReleaseFrame(pClass->stModDesc.devId, pClass->stModDesc.chnId, &stFrm, &stAecFrm);
            }
        }
        break;
#endif
#if INTERFACE_VENC
        case E_SYS_MOD_VENC:
        {
            s32Fd = MI_VENC_GetFd((MI_VENC_CHN)stChnOutputPort.u32ChnId);
            if (s32Fd < 0)
            {
                return 0;
            }

            FD_ZERO(&read_fds);
            FD_SET(s32Fd, &read_fds);
            tv.tv_sec = 0;
            tv.tv_usec = 10 * 1000;
            s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
            if (s32Ret < 0)
            {
                MI_VENC_CloseFd(s32Fd);

                return NULL;
            }
            else if (0 == s32Ret)
            {
                MI_VENC_CloseFd(s32Fd);

                return NULL;
            }
            else
            {
                Venc *pVencClass = dynamic_cast<Venc*>(pClass);
                memset(&stStream, 0, sizeof(stStream));
                memset(stPack, 0, sizeof(stPack));
                stStream.pstPack = stPack;
                s32Ret = MI_VENC_Query(stChnOutputPort.u32ChnId, &stStat);
                if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
                {
                    MI_VENC_CloseFd(s32Fd);
                    return NULL;
                }
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = MI_VENC_GetStream(stChnOutputPort.u32ChnId, &stStream, 40);
                if(MI_SUCCESS == s32Ret)
                {
                    stVencInfo_t stVencInfo;
                    pVencClass->GetInfo(stVencInfo);
                    switch(stVencInfo.intEncodeType)
                    {
                        case E_MI_VENC_MODTYPE_H264E:
                        {
                            stStreamInfo.eStreamType =  E_STREAM_H264;
                        }
                        break;
                        case E_MI_VENC_MODTYPE_H265E:
                        {
                            stStreamInfo.eStreamType =  E_STREAM_H265;
                        }
                        break;
                        case E_MI_VENC_MODTYPE_JPEGE:
                        {
                            stStreamInfo.eStreamType =  E_STREAM_JPEG;
                        }
                        break;
                        default:
                            stStreamInfo.eStreamType =  E_STREAM_H264;
                            break;
                    }
                    stEsPackage_t stEsPacket[16];
                    stStreamInfo.stCodecInfo.uintPackCnt = stStream.u32PackCount;
                    for (MI_U8 i = 0; i < stStream.u32PackCount; i++)
                    {
                        stEsPacket[i].uintDataSize = stStream.pstPack[i].u32Len;
                        stEsPacket[i].pData = (char*)stStream.pstPack[i].pu8Addr;
                    }
                    stStreamInfo.stCodecInfo.pDataAddr = stEsPacket;
                    //printf("Receiver %p Get venc chn %d and send to port %d this is %s\n", pReceiver, stChnOutputPort.u32ChnId, pReceiver->uintPort, pClass->stModDesc.modKeyString.c_str());
                    pClass->Send(pReceiver->uintPort, &stStreamInfo, sizeof(stStreamInfo));
                    MI_VENC_ReleaseStream(stChnOutputPort.u32ChnId, &stStream);
                }
            }
            MI_VENC_CloseFd(s32Fd);
        }
        break;
#endif
        default:
            printf("Do not support this mod %d now\n", pClass->stModDesc.modId);
            break;
    }
    return NULL;
}
#if INTERFACE_VDEC
static MI_U64 _GetPts(MI_U32 u32FrameRate)
{
    if (0 == u32FrameRate)
    {
        return (MI_U64)(-1);
    }

    return (MI_U64)(1000 / u32FrameRate);
}
#endif
void Sys::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    int y_size = 0;
    int uv_size = 0;
    int yuv_size = 0;
    stStreamInfo_t *pStreamInfo = NULL;
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_ChnPort_t stSysChnInputPort;
    Sys *pInstance = NULL;

    pInstance = (Sys *)pUsrData;
    if (pInstance->bExtract)
        return;
    if (sizeof(stStreamInfo_t) == dataSize)
    {
        pStreamInfo = (stStreamInfo_t *)pData; 
        switch (pStreamInfo->eStreamType)
        {
            case E_STREAM_YUV420:
            case E_STREAM_YUV422:
            {
                memset(&stBufConf ,  0 , sizeof(stBufConf));
                memset(&stBufInfo ,  0 , sizeof(stBufInfo));
                memset(&stSysChnInputPort, 0, sizeof(stSysChnInputPort));

                stSysChnInputPort.eModId = (MI_ModuleId_e)pInstance->stModDesc.modId;
                stSysChnInputPort.u32DevId = pInstance->stModDesc.devId;
                stSysChnInputPort.u32ChnId = pInstance->stModDesc.chnId;
                stSysChnInputPort.u32PortId = portId;

                MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
                stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
                stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                stBufConf.stFrameCfg.u16Width = pStreamInfo->streamWidth;
                stBufConf.stFrameCfg.u16Height = pStreamInfo->streamHeight;
                stBufConf.stFrameCfg.eFormat = (MI_SYS_PixelFormat_e)pStreamInfo->eStreamType;

                if(MI_SUCCESS  == (s32Ret = MI_SYS_ChnInputPortGetBuf(&stSysChnInputPort, &stBufConf, &stBufInfo, &hHandle, 0)))
                {
                    if(stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                    {
                        y_size = pStreamInfo->streamWidth*pStreamInfo->streamHeight;
                        uv_size = y_size/2;
                        memcpy(stBufInfo.stFrameData.pVirAddr[0], pStreamInfo->stYuvInfo.pYdataAddr, y_size);
                        memcpy(stBufInfo.stFrameData.pVirAddr[1], pStreamInfo->stYuvInfo.pUvDataAddr, uv_size);
                    }
                    else if(stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
                    {
                        yuv_size = pStreamInfo->streamWidth*pStreamInfo->streamHeight*2;
                        memcpy(stBufInfo.stFrameData.pVirAddr[0], pStreamInfo->stYuvInfo.pYuvDataAddr, yuv_size);
                    }
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                    if(s32Ret != MI_SUCCESS)
                    {
                        printf("put buf err is %x\n", s32Ret);
                    }
                }
                else
                {
                    printf("get port buf err is %x\n", s32Ret);
                }
            }
            break;
            case E_STREAM_H264:
            case E_STREAM_H265:
            {
#if INTERFACE_VDEC
                unsigned int modId = FindBlockId(pInstance->stModDesc.modKeyString);

                if(modId == E_SYS_MOD_VDEC)
                {
                    unsigned int i=0;
                    MI_U64 u64Pts = 0;
                    MI_VDEC_VideoStream_t stVdecStream;
                    for(i=0; i<pStreamInfo->stCodecInfo.uintPackCnt;i++)
                    {
                        memset(&stVdecStream, 0x0, sizeof(stVdecStream));
                        stVdecStream.pu8Addr = (MI_U8*)pStreamInfo->stCodecInfo.pDataAddr[i].pData;
                        stVdecStream.u32Len = pStreamInfo->stCodecInfo.pDataAddr[i].uintDataSize;
                        stVdecStream.u64PTS = u64Pts + _GetPts(((MI_U32)30));
                        stVdecStream.bEndOfFrame = 1;
                        stVdecStream.bEndOfStream = 0;
                        //printf("Addr %p len %d\n", stVdecStream.pu8Addr, stVdecStream.u32Len);
                        s32Ret = MI_VDEC_SendStream(pInstance->stModDesc.chnId, &stVdecStream, 20);
                        if (MI_SUCCESS != s32Ret)
                        {
                            printf("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", pInstance->stModDesc.chnId, s32Ret);
                        }
                    }
                }
#endif
            }
            break;
            default:
                //printf("Not support!!\n");
                //assert(0);
                break;
        }
    }
    else
    {
        assert(0);
    }
}

