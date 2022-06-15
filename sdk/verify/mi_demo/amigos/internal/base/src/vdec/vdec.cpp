#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <string>

#include "vdec.h"
#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_common.h"


Vdec::Vdec()
{
}
Vdec::~Vdec()
{
}
void Vdec::LoadDb()
{
    std::map<unsigned int, stModOutputInfo_t>::iterator itVdecOut;
    stDecOutInfo_t stDecOutInfo;

    stVdecInfo.picWidth = GetIniInt(stModDesc.modKeyString,"VDEC_W");
    stVdecInfo.picHeight = GetIniInt(stModDesc.modKeyString,"VDEC_H");
    stVdecInfo.decodeType = GetIniInt(stModDesc.modKeyString,"DE_TYPE");
    stVdecInfo.dpBufMode = GetIniInt(stModDesc.modKeyString,"BUF_MODE");
    stVdecInfo.refFrameNum = GetIniInt(stModDesc.modKeyString,"REF_FRAME");
    stVdecInfo.bitstreamSize = GetIniInt(stModDesc.modKeyString,"BIT_STREAM_BUFFER");
    for (itVdecOut = mapModOutputInfo.begin(); itVdecOut != mapModOutputInfo.end(); itVdecOut++)
    {
        memset(&stDecOutInfo, 0, sizeof(stDecOutInfo_t));
        stDecOutInfo.intPortId = itVdecOut->second.curPortId;
        stDecOutInfo.intDecOutWidth = GetIniInt(itVdecOut->second.curIoKeyString, "VID_W");
        stDecOutInfo.intDecOutHeight = GetIniInt(itVdecOut->second.curIoKeyString, "VID_H");
        if (stDecOutInfo.intDecOutWidth > stVdecInfo.picWidth)
        {
            stDecOutInfo.intDecOutWidth = stVdecInfo.picWidth;
        }
        if (stDecOutInfo.intDecOutHeight > stVdecInfo.picHeight)
        {
            stDecOutInfo.intDecOutHeight = stVdecInfo.picHeight;
        }
        vDecOutInfo.push_back(stDecOutInfo);
    }
}
void Vdec::Init()
{
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    std::vector<stDecOutInfo_t>::iterator itVdecOut;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = stVdecInfo.refFrameNum;
    stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize    = stVdecInfo.bitstreamSize * 1024 * 1024;
    stVdecChnAttr.u32PicWidth   = stVdecInfo.picWidth;
    stVdecChnAttr.u32PicHeight  = stVdecInfo.picHeight;
    stVdecChnAttr.u32Priority   = 0;
    stVdecChnAttr.eCodecType    = (MI_VDEC_CodecType_e)stVdecInfo.decodeType;
    stVdecChnAttr.eDpbBufMode = (MI_VDEC_DPB_BufMode_e)stVdecInfo.dpBufMode;


    MI_VDEC_CreateChn(stModDesc.chnId, &stVdecChnAttr);
    MI_VDEC_StartChn(stModDesc.chnId);

    for (itVdecOut = vDecOutInfo.begin(); itVdecOut != vDecOutInfo.end(); itVdecOut++)
    {
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width = itVdecOut->intDecOutWidth;
        stOutputPortAttr.u16Height = itVdecOut->intDecOutHeight;

        MI_VDEC_SetOutputPortAttr((MI_VDEC_CHN)stModDesc.chnId, &stOutputPortAttr);
    }
}
void Vdec::Deinit()
{
    MI_VDEC_StopChn((MI_VDEC_CHN)stModDesc.chnId);
    MI_VDEC_DestroyChn((MI_VDEC_CHN)stModDesc.chnId);
}
