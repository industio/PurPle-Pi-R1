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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "file.h"

File::File()
{
}
File::~File()
{
}
void File::LoadDb()
{
    stFileOutInfo_t stFileOutput;
    stFileInInfo_t stFileInInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapFileIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapFileOut;

    for (itMapFileIn = mapModInputInfo.begin(); itMapFileIn != mapModInputInfo.end(); itMapFileIn++)
    {
        stFileInInfo.fileName = GetIniString(itMapFileIn->second.curIoKeyString, "FILE_WRITE_PATH");
        mapInputWrFile[itMapFileIn->second.curPortId] = stFileInInfo;
    }
    for (itMapFileOut = mapModOutputInfo.begin(); itMapFileOut != mapModOutputInfo.end(); itMapFileOut++)
    {
        stFileOutput.fileName = GetIniString(itMapFileOut->second.curIoKeyString, "FILE_READ_PATH");
        stFileOutput.intFileOutWidth = GetIniInt(itMapFileOut->second.curIoKeyString,  "VID_W");
        stFileOutput.intFileOutHeight = GetIniInt(itMapFileOut->second.curIoKeyString, "VID_H");
        stFileOutput.intFileFmt= GetIniInt(itMapFileOut->second.curIoKeyString, "VID_FMT");
        mapOutputRdFile[itMapFileOut->second.curPortId] = stFileOutput;
    }
}
void File::Init()
{
    std::map<int, stFileOutInfo_t>::iterator itFileOut;

    for (itFileOut = mapOutputRdFile.begin(); itFileOut != mapOutputRdFile.end(); itFileOut++)
    {
        if ((access(itFileOut->second.fileName.c_str(), F_OK)) ==-1)
        {
            printf("not access %s file!\n", itFileOut->second.fileName.c_str());
            return;
        }
        
        itFileOut->second.intreadFd = open(itFileOut->second.fileName.c_str(), O_RDONLY);
        if(itFileOut->second.intreadFd < 0)
        {
            printf("read_file: %s. open fail\n", itFileOut->second.fileName.c_str());
            return;
        }
    }
}
void File::Deinit()
{
    std::map<int, stFileOutInfo_t>::iterator it;
    for(it=mapOutputRdFile.begin();it != mapOutputRdFile.end();++it)
    {
        close(it->second.intreadFd);
    }
}
void File::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);

    std::map<int, stFileInInfo_t>::iterator it;

    printf("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    it = mapInputWrFile.find(stIn.curPortId);
    if (it != mapInputWrFile.end())
    {
        it->second.intreadFd = open(it->second.fileName.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0777);
        if (it->second.intreadFd < 0)
        {
            printf("dest_file: %s.\n", it->second.fileName.c_str());
            perror("open");
            return;
        }
        CreateReceiver(stIn.curPortId, DataReceiver, NULL, NULL, (void *)it->second.intreadFd);
        StartReceiver(stIn.curPortId);
    }
}
void File::UnBindBlock(stModInputInfo_t & stIn)
{
   std::map<int, stFileInInfo_t>::iterator it;

    it = mapInputWrFile.find(stIn.curPortId);
    if (it != mapInputWrFile.end())
    {
        StopReceiver(stIn.curPortId);
        DestroyReceiver(stIn.curPortId);
        close(it->second.intreadFd);
    }
}

void File::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    int intFd = 0;
    stStreamInfo_t *pStreamInfo = NULL;

    intFd = (int)pUsrData;
    if (sizeof(stStreamInfo_t) == dataSize)
    {
        pStreamInfo = (stStreamInfo_t*)pData;
        
        switch (pStreamInfo->eStreamType)
        {
            case E_STREAM_YUV420:
            {
                int yuv_size = pStreamInfo->streamWidth * pStreamInfo->streamHeight * 2;
                char *dst_buf;

                dst_buf = pStreamInfo->stYuvInfo.pYuvDataAddr;
                if(write(intFd, dst_buf, yuv_size) != yuv_size)
                {
                    printf("write yuv data err!\n");
                    return;
                }
            }
            break;
            case E_STREAM_YUV422:
            {
                int y_size = pStreamInfo->streamWidth * pStreamInfo->streamHeight;
                int uv_size = y_size/2;
                char *dst_buf;

                dst_buf = pStreamInfo->stYuvInfo.pYdataAddr;
                if(write(intFd, dst_buf, y_size) != y_size)
                {
                    printf("write yuv data err!\n");
                    return;
                }
                dst_buf = pStreamInfo->stYuvInfo.pUvDataAddr;
                if(write(intFd, dst_buf, uv_size) != uv_size)
                {
                    printf("write uvdata err!\n");
                    return;
                }
            }
            break;
            case E_STREAM_H264:
            case E_STREAM_H265:
            {
                for (MI_U8 i = 0; i < pStreamInfo->stCodecInfo.uintPackCnt; i++)
                {
                    write(intFd, (void *)pStreamInfo->stCodecInfo.pDataAddr[i].pData, pStreamInfo->stCodecInfo.pDataAddr[i].uintDataSize);
                }
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

void *File::SenderState(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stUsrData)
{
    stSenderState_t stSenderState;
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    File *pClass = dynamic_cast<File *>(pReceiver->pSysClass);

    assert(stUsrData.pUserData);
    assert(stUsrData.u32UserDataSize == sizeof(stSenderState_t));
    stSenderState = *((stSenderState_t *)stUsrData.pUserData);
    assert(stSenderState.pData);
    pClass->State(pReceiver->uintPort, stSenderState.eState, *((stReceiverPortDesc_t *)stSenderState.pData));

    return NULL;
}

int File::CreateSender(unsigned int outPortId)
{
    ST_TEM_ATTR stTemAttr;
    stReceiverDesc_t stDest;

    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = SenderState;
    stTemAttr.fpThreadWaitTimeOut = SenderMonitor;
    stTemAttr.u32ThreadTimeoutMs = 35;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = (void *)&(mapRecevier[outPortId]);
    stTemAttr.stTemBuf.u32TemBufferSize = 0;
    TemOpen(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stTemAttr);

    return 0;
}
MI_S32 GetOneFrameYUV420(int srcFd, char *pYData, char *pUvData, int ySize, int uvSize)
{
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) ==0)
    {
        lseek(srcFd, 0, SEEK_SET);
        current = lseek(srcFd,0L, SEEK_CUR);
        end = lseek(srcFd,0L, SEEK_END);
    }
    
    if((end - current) < (ySize + uvSize))
    {
        return -1;
    }
    
    lseek(srcFd, current, SEEK_SET);
    if (read(srcFd, pYData, ySize) == ySize 
        && read(srcFd, pUvData, uvSize) == uvSize)
    {
        return 1;
    } 

    return -1;
}

MI_S32 GetOneFrameYUV422(int srcFd, char *pYuvData, int yuvSize)
{
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) ==0)
    {
        lseek(srcFd, 0, SEEK_SET);
        current = lseek(srcFd,0L, SEEK_CUR);
        end = lseek(srcFd,0L, SEEK_END);
    }

    if ((end - current) < yuvSize)
    {
        return -1;
    }
   
    lseek(srcFd, current, SEEK_SET);
    if (read(srcFd, pYuvData, yuvSize) == yuvSize)
    {
        return 1;
    }
    
    return 0;
}

void * File::SenderMonitor(ST_TEM_BUFFER stBuf)
{
    int readfp = -1;
    //MI_U32 u32Pos = 0;
    MI_S32 s32Len = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U32 u32FrameLen = 0;
    MI_U8 au8Header[32] = {0};


    int ret = -1;
    int y_size = 0;
    int uv_size = 0;
    int yuvSize = 0;
    char *pYdata = NULL;
    char *pUvdata = NULL;
    char *pYuvdata = NULL;
    stStreamInfo_t stFileStream;
    stEsPackage_t stEsPacket[1];

    
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    File *pSendClass = dynamic_cast<File *>(pReceiver->pSysClass);

    readfp = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intreadFd;
    if(readfp < 0)
    {
        printf("file send file handle is null!\n");
        return NULL;
    }

    memset(&stFileStream, 0, sizeof(stFileStream));
    stFileStream.eStreamType = (E_STREAM_TYPE)((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileFmt;
    stFileStream.streamWidth = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
    stFileStream.streamHeight = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
    
    switch (stFileStream.eStreamType)
    {
        case E_STREAM_YUV420:
        case E_STREAM_YUV422:
        {
            if(stFileStream.eStreamType == E_STREAM_YUV420)
            {
                y_size  = stFileStream.streamWidth*stFileStream.streamHeight;
                uv_size  = y_size/2;

                pYdata = (char*)malloc(y_size);
                pUvdata = (char*)malloc(uv_size);
                if(pYdata == NULL || pUvdata == NULL)
                {
                    goto yuv_free_buf;
                }
                ret =  GetOneFrameYUV420(readfp, pYdata, pUvdata, y_size, uv_size);
                if(ret == 1)
                {
                    stFileStream.stYuvInfo.pYdataAddr  = pYdata;
                    stFileStream.stYuvInfo.pUvDataAddr = pUvdata;
                    pSendClass->Send(pReceiver->uintPort, &stFileStream, sizeof(stFileStream));
                }
                
            }
            else if(stFileStream.eStreamType == E_STREAM_YUV422)
            {
                yuvSize = stFileStream.streamWidth*stFileStream.streamHeight*2;
                pYuvdata = (char*)malloc(yuvSize);

                if(pYuvdata == NULL)
                {
                    return NULL;
                }
                ret =  GetOneFrameYUV422(readfp, pYuvdata, yuvSize);
                if(ret == 1)
                {
                    stFileStream.stYuvInfo.pYuvDataAddr  = pYuvdata;
                    pSendClass->Send(pReceiver->uintPort, &stFileStream, sizeof(stFileStream));
                }

            }
yuv_free_buf:
            if(pYdata != NULL)
            {
                free(pYdata);
            }
            
            if(pUvdata != NULL)
            {
                free(pUvdata);
            }

            if(pYuvdata != NULL)
            {
                free(pYuvdata);
            }
        }
        break;
        case E_STREAM_H264:
        case E_STREAM_H265:
        {
            memset(&stEsPacket, 0, sizeof(stEsPacket));
            memset(au8Header, 0, 16);
            pu8Buf = (MI_U8 *)malloc(NALU_PACKET_SIZE);
            if(pu8Buf == NULL)
            {
                return NULL;
            }
            //u32Pos = lseek(readfp, 0, SEEK_CUR);
            s32Len = read(readfp, au8Header, 16);

            if(s32Len <= 0)
            {
                lseek(readfp, 0, SEEK_SET);
                goto codec_free_buf;
            }

            u32FrameLen = MI_U32VALUE(au8Header, 4);
            if(u32FrameLen > NALU_PACKET_SIZE)
            {
                lseek(readfp, 0, SEEK_SET);
                goto codec_free_buf;
            }
    
            s32Len = read(readfp, pu8Buf, u32FrameLen);
            if(s32Len <= 0)
            {
                lseek(readfp, 0, SEEK_SET);
                goto codec_free_buf;
            }

            stEsPacket[0].uintDataSize = s32Len;
            stEsPacket[0].pData = (char*)pu8Buf;
            stFileStream.stCodecInfo.uintPackCnt = 1;
            stFileStream.stCodecInfo.pDataAddr = stEsPacket;
            pSendClass->Send(pReceiver->uintPort, &stFileStream, sizeof(stFileStream)); 

codec_free_buf:
            if(pu8Buf != NULL)
            {
                free(pu8Buf);
            }
        }
        break;
        default:
            break;
    }

    return NULL;
}

