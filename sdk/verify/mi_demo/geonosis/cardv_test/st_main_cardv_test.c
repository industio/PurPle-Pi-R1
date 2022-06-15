#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "st_common.h"

#include "SAT070CP50_1024x600.h"

#define MAX_ONE_FRM_SIZE (2 * 1024 * 1024)
#define DROP_SLICE_ID (3)
#define VDEC_CHN_ID     0

MI_U32 g_u32InWidth   = 1920;
MI_U32 g_u32InHeight  = 1080;
MI_U32 g_u32OutWidth    = 640;
MI_U32 g_u32OutHeight   = 480;
MI_U32 g_u32RefFrmNum = 5;
MI_VDEC_CodecType_e g_eCodecType = E_MI_VDEC_CODEC_TYPE_H264;


#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)

typedef struct ReOrderSlice_s {
    MI_U8 *pos;
    MI_U32 len;
} ReOrderSlice_t;

typedef struct
{
    int startcodeprefix_len;
    unsigned int len;
    unsigned int max_size;
    char *buf;
    unsigned short lost_packets;
} NALU_t;

ReOrderSlice_t _stVdecSlice[100];
MI_U32 _u32SliceIdx = 0;
char _pFileName[256];
pthread_t _pthrPushStream = 0;
MI_BOOL _bExit = FALSE;
MI_VDEC_CodecType_e _eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
MI_BOOL _bFrmMode = TRUE;
MI_BOOL _bReOrderSlice = FALSE;
MI_BOOL _bSleep = FALSE;
MI_BOOL _bDropFrm = FALSE;

static int FindStartCode2 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    {
        printf("AllocNALU: n");
        exit(0);
    }
    n->max_size=buffersize;
    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
    {
        free (n);
        printf ("AllocNALU: n->buf");
        exit(0);
    }
    return n;
}

void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
}

int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn, FILE *fp)
{
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;
    int info2 = 0, info3 = 0;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
        printf ("GetAnnexbNALU: Could not allocate Buf memory\n");
    nalu->startcodeprefix_len=3;
    if (3 != fread (Buf, 1, 3, fp))
    {
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2 (Buf);
    if(info2 != 1)
    {
        if(1 != fread(Buf+3, 1, 1, fp))
        {
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;
    while (!StartCodeFound)
    {
        if (feof (fp))
        {
            nalu->len = (pos-1)-nalu->startcodeprefix_len;
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            free(Buf);
            fseek(fp, 0, 0);
            return pos-1;
        }
        Buf[pos++] = fgetc (fp);
        info3 = FindStartCode3(&Buf[pos-4]);
        if(info3 != 1)
            info2 = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }
    rewind = (info3 == 1) ? -4 : -3;
    if (0 != fseek (fp, rewind, SEEK_CUR))
    {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }
    nalu->len = (pos+rewind);
    memcpy (nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos+rewind);
}

MI_U64 GetOSTime(void)
{
    MI_U64 u64CurTime = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    u64CurTime = ((unsigned long long)(tv.tv_sec))*1000 + tv.tv_usec/1000;
    return u64CurTime;
}

void *ST_VdecSendStream(void *args)
{
    MI_VDEC_CHN vdecChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32TimeOutMs = 20;
    MI_U32 u32WrPtr = 0;
    MI_S32 s32NaluType = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U8 *pSlice2FrmBuf = NULL;
    MI_U8 *pSliceReOrder = NULL;
    MI_U64 u64Pts = 0;
    MI_U64 u64FisrtSlicePushTime = 0;
    MI_BOOL bFirstFrm = FALSE;
    MI_BOOL frameType = 0;
    MI_U32 u32SliceCount = 0;
    MI_U32 u32FpBackLen = 0; // if send stream failed, file pointer back length
    FILE *fReadFile = NULL;
    NALU_t *pstNalu;
    MI_VDEC_VideoStream_t stVdecStream;

    pSlice2FrmBuf = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);

    if (_bReOrderSlice) {
        pSliceReOrder = (unsigned char *)malloc(MAX_ONE_FRM_SIZE);
    }

    pstNalu = AllocNALU(MAX_ONE_FRM_SIZE);
    prctl(PR_SET_NAME, "push_thrd_0");
    fReadFile = fopen(_pFileName, "rb"); //ES
    if (!fReadFile)
    {
        ST_ERR("Open %s failed!\n", _pFileName);
        return NULL;
    }

    printf("open %s success, vdec chn:%d\n", _pFileName, vdecChn);
    pu8Buf = malloc(MAX_ONE_FRM_SIZE);
    while (!_bExit)
    {
        GetAnnexbNALU(pstNalu, vdecChn, fReadFile);
        stVdecStream.pu8Addr = (MI_U8 *)pstNalu->buf;
        if(9 == pstNalu->len
            && 0 == *(pstNalu->buf)
            && 0 == *(pstNalu->buf+1)
            && 0 == *(pstNalu->buf+2)
            && 1 == *(pstNalu->buf+3)
            && 0x68 == *(pstNalu->buf+4)
            && 0 == *(pstNalu->buf+pstNalu->len-1))
        {
            stVdecStream.u32Len = 8;
        }
        else {
            stVdecStream.u32Len = pstNalu->len;
        }
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len; //back length
        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && (0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4]))
        {
            //usleep(u32SendStreamDelayMs * 1000);
        }


        if (_eCodecType == E_MI_VDEC_CODEC_TYPE_H265) {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[6] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[4] & 0x7E) >> 1;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = (stVdecStream.pu8Addr[3] & 0x7E) >> 1;
            }

            if (s32NaluType <= 31) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
        } else {
            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]) {
                bFirstFrm = (stVdecStream.pu8Addr[5] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[4] & 0xF;
            }

            if (0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1] && 0x01 == stVdecStream.pu8Addr[2]) {
                bFirstFrm = (stVdecStream.pu8Addr[4] & 0x80);
                s32NaluType = stVdecStream.pu8Addr[3] & 0xF;
            }
            if (1 <= s32NaluType && s32NaluType <= 5) {
                ///frame type
                frameType = 1;
            } else {
                frameType = 0;
            }
        }

        if (bFirstFrm) {
            u32SliceCount = 0;
        }

        if (u64FisrtSlicePushTime == 0) {
            u64FisrtSlicePushTime = GetOSTime();
        }

        if (_bDropFrm && frameType && (GetOSTime() - u64FisrtSlicePushTime > 3000) && (u32SliceCount == DROP_SLICE_ID)) {
            printf("drop slice, id=%d, 0x%02x, type:%d\n", u32SliceCount, stVdecStream.pu8Addr[4], s32NaluType);
            u32SliceCount++;
            continue;
        }

        if (_bFrmMode) {
            if (u32WrPtr && bFirstFrm) {
                MI_U8 *pTmp = stVdecStream.pu8Addr;
                MI_U32 u32TmpLen = stVdecStream.u32Len;

                stVdecStream.pu8Addr = pSlice2FrmBuf;
                stVdecStream.u32Len = u32WrPtr;

                if (_bReOrderSlice && frameType && (GetOSTime() - u64FisrtSlicePushTime > 3000)) {
                    int WLen = 0;
                    for (int sliceID = 0; sliceID < _u32SliceIdx; ++sliceID) {
                        if (sliceID == 0) {
                            memcpy(pSliceReOrder, _stVdecSlice[0].pos, _stVdecSlice[0].len);
                            WLen += _stVdecSlice[0].len;
                        } else {
                            memcpy(pSliceReOrder + WLen, _stVdecSlice[_u32SliceIdx - sliceID].pos, _stVdecSlice[_u32SliceIdx - sliceID].len);
                            WLen += _stVdecSlice[_u32SliceIdx - sliceID].len;
                        }
                    }

                    stVdecStream.pu8Addr = pSliceReOrder;
                    printf("reorder done...\n");
                }

                if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
                {
                    printf("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                    fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                } else {
                    stVdecStream.pu8Addr = pTmp;
                    stVdecStream.u32Len = u32TmpLen;
                    u32WrPtr = 0;
                    if (_bReOrderSlice) {
                        _u32SliceIdx = 0;
                    }
                }

                usleep(30 * 1000);
            }

            memcpy(pSlice2FrmBuf + u32WrPtr, stVdecStream.pu8Addr, stVdecStream.u32Len);
            if (_bReOrderSlice) {
                _stVdecSlice[_u32SliceIdx].pos = pSlice2FrmBuf + u32WrPtr;
                _stVdecSlice[_u32SliceIdx].len = stVdecStream.u32Len;
                _u32SliceIdx++;
            }
            u32WrPtr += stVdecStream.u32Len;
        } else {
            if (_bSleep) {
            }

            if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
            {
                //ST_ERR("chn[%d]: MI_VDEC_SendStream %d fail, 0x%X\n", vdecChn, stVdecStream.u32Len, s32Ret);
                fseek(fReadFile, - u32FpBackLen, SEEK_CUR);
                usleep(30 * 1000);
            }
        }

        u32SliceCount++;
    }

    printf("\n\n");
    free(pu8Buf);
    FreeNALU(pstNalu);
    fclose(fReadFile);
    free(pSlice2FrmBuf);
    printf("End----------------------%d------------------End\n", vdecChn);

    return NULL;
}

int ST_Init(void)
{
    //init sys
    MI_SYS_Init();

    //init vdec
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    MI_VDEC_InitParam_t stInitParm;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType   = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = g_u32RefFrmNum;
    stVdecChnAttr.eVideoMode   = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize   = 1 * 1920 * 1080;
    stVdecChnAttr.u32PicWidth  = g_u32InWidth;
    stVdecChnAttr.u32PicHeight = g_u32InHeight;
    stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority  = 0;

    memset(&stInitParm, 0x0, sizeof(stInitParm));
    stInitParm.bDisableLowLatency = TRUE;
    MI_VDEC_InitDev(&stInitParm);
    STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));

    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width  = g_u32OutWidth;
    stOutputPortAttr.u16Height = g_u32OutHeight;
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(0, &stOutputPortAttr));

    MI_SYS_ChnPort_t  stChnPort;
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = stVdecChn;
    stChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));

    //init disp
    MI_PANEL_LinkType_e eLinkType;
    MI_DISP_PubAttr_t stDispPubAttr;

    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.u32BgColor = YUYV_BLACK;

    stDispPubAttr.stSyncInfo.u16Vact = stPanelParam.u16Height;
    stDispPubAttr.stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
    stDispPubAttr.stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
    stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
    stDispPubAttr.stSyncInfo.u16Hact = stPanelParam.u16Width;
    stDispPubAttr.stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
    stDispPubAttr.stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
    stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
    stDispPubAttr.stSyncInfo.u16Bvact = 0;
    stDispPubAttr.stSyncInfo.u16Bvbb = 0;
    stDispPubAttr.stSyncInfo.u16Bvfb = 0;
    stDispPubAttr.stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
    stDispPubAttr.stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
    stDispPubAttr.stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
    stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;

#if USE_MIPI
    eLinkType = E_MI_PNL_LINK_MIPI_DSI;
#else
    eLinkType = E_MI_PNL_LINK_TTL;
#endif

    MI_DISP_SetPubAttr(0, &stDispPubAttr);
    MI_DISP_Enable(0);
    MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    MI_DISP_InputPortAttr_t stInputPortAttr;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));

    stInputPortAttr.u16SrcWidth = g_u32OutWidth;
    stInputPortAttr.u16SrcHeight = g_u32OutHeight;
    stInputPortAttr.stDispWin.u16X = 0;
    stInputPortAttr.stDispWin.u16Y = 0;
    stInputPortAttr.stDispWin.u16Width = g_u32OutWidth;
    stInputPortAttr.stDispWin.u16Height = g_u32OutHeight;

    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    MI_PANEL_Init(eLinkType);
    MI_PANEL_SetPanelParam(&stPanelParam);
    if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
    {
#if USE_MIPI
        MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
#endif
    }

    //bind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
    MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 30, 30);
    return 0;
}

int ST_DeInit(void)
{
    //Unbind vdec 2 disp
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;

    STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));

    STCHECKRESULT(MI_VDEC_StopChn(0));
    STCHECKRESULT(MI_VDEC_DestroyChn(0));

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, 0);
    MI_DISP_Disable(0);
    MI_PANEL_DeInit();

    MI_SYS_Exit();
    return 0;
}

void ST_VDEC_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit\n");

        ST_DeInit();
        exit(0);
    }
}

MI_U32 _u32TestCase = 0;
void ST_TestCase(void)
{
    printf("select test case:\n");
    printf("0:\t frame mode\n");
    printf("1:\t slice mode\n");
    printf("2:\t reorder slice\n");
    printf("3:\t drop slice\n");
    printf("4:\t sleep 1 second\n");
    scanf("%d", &_u32TestCase);

    switch (_u32TestCase) {
        case 0:
            _bFrmMode = TRUE;
            break;
        case 1:
            _bFrmMode = FALSE;
            break;
        case 2:
            _bFrmMode = TRUE;
            _bReOrderSlice = TRUE;
            break;
        case 3:
            _bDropFrm = TRUE;
            break;
        case 4:
            _bSleep = TRUE;
            break;
        default:
            exit(0);
    }
}

int main(int argc, char **argv)
{
    struct sigaction sigAction;

    sigAction.sa_handler = ST_VDEC_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);

    /************************************************
    step1:  init sys
    *************************************************/

    if (argc < 5) {
        printf("exec like: ./prog_cardv_test filename h264/h265 frmwidth frmheight [refrm_num dispwitdh dispheight]\n");
        exit(0);
    }

    memcpy(_pFileName, argv[1], strlen(argv[1]));
    if (0 == strncmp("h264", argv[2], 4)) {
        g_eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
     } else if (0 == strncmp("h265", argv[2], 4)) {
        g_eCodecType = E_MI_VDEC_CODEC_TYPE_H265;
    } else {
        printf("only support h264/h265 error\n");
        exit(1);
    }

    g_u32InWidth = atoi(argv[3]);
    g_u32InHeight = atoi(argv[4]);

    if (argc > 5) {
        g_u32RefFrmNum = atoi(argv[5]);
    }

    if (argc > 7) {
        g_u32OutWidth = atoi(argv[6]);
        g_u32OutHeight = atoi(argv[7]);
    }

#if (0)
    ST_TestCase();
    printf("file:%s, frm:%d, drop:%d, sleep:%d\n",
        _pFileName, _bFrmMode, _bDropFrm, _bSleep);
#endif
    ST_Init();

    _bExit = FALSE;
    pthread_create(&_pthrPushStream, NULL, ST_VdecSendStream, NULL);

    while(1)
    {
        sleep(1);
    }

    ST_DeInit();
    return 0;
}
