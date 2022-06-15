#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*******************************************************************
 *H264码流结构：
 *Start Code: 00 00 00 01
 *SPS:
 *PPS:
 *I_SLICE:
 *P_SLICE:
 *
 *H265码流结构：
 *Start Code: 00 00 00 01
 *SPS:
 *VPS:
 *PPS:
 *I_SLICE:
 *P_SLICE:
 *******************************************************************/

#define SUCCESS 0
#define FAILURE -1
#define MAX_FILE_NAME_LEN 128
#define HEAD_INFO_LEN 16
#define MAX_NAIL_BUFF_SIZE (1024*1024*4)

typedef char 				S8;
typedef unsigned char 		U8;
typedef int  				S32;
typedef unsigned int  		U32;
typedef long long 			S64;
typedef unsigned long long 	U64;

typedef enum
{
	FALSE = 0,
	TRUE
} BOOL;


typedef enum
{
    E_NALU_TYPE_SLICE    = 1,
    E_NALU_TYPE_DPA      = 2,
    E_NALU_TYPE_DPB      = 3,
    E_NALU_TYPE_DPC      = 4,
    E_NALU_TYPE_IDR      = 5,
    E_NALU_TYPE_SEI      = 6,
    E_NALU_TYPE_SPS      = 7,
    E_NALU_TYPE_PPS      = 8,
    E_NALU_TYPE_AUD      = 9,
    E_NALU_TYPE_EOSEQ    = 10,
    E_NALU_TYPE_EOSTREAM = 11,
    E_NALU_TYPE_FILL     = 12,
} NALU_TYPE_e;

typedef enum {
    E_NALU_PRIORITY_DISPOSABLE = 0,
    E_NALU_PRIORITY_LOW        = 1,
    E_NALU_PRIORITY_HIGH       = 2,
    E_NALU_PRIORITY_HIGHEST    = 3
} NALU_PRIORITY_e;

typedef struct NALU_s
{
    S32 startcodeprefix_len; //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    U32 len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    U32 max_size;            //! Nal Unit Buffer size
    S32 forbidden_bit;       //! should be always FALSE
    S32 nal_reference_idc;   //! NALU_PRIORITY_xxxx
    S32 nal_unit_type;       //! NALU_TYPE_xxxx
    S8  *buf;                //! contains the first byte followed by the EBSP
} NALU_t;

#define CHECK_NULL_POINTER(func, pointer) \
    do\
	{\
	    if(pointer==NULL) \
	    {\
            printf("%s Parameter NULL!Line: %d\n", func, __LINE__);\
            return FAILURE;\
	    }\
	}while(0);

//ref, no use
#define STARTCODE_TEST(i,length,src) \
	if(i + 2 < length && src[i + 1] == 0 && src[i + 2]<= 3)\
	{\
     	if(src[i + 2] != 3)\
		{\
    		/* startcode, so we must bepast the end*/ \
     		length = i;\
   		}\
        break;\
	}


BOOL bInfo2 = FALSE;
BOOL bInfo3 = TRUE;


static S32 WrtiteHeadInfo(FILE *pEsFile, U32 u32FrmLen)
{
	U8 au8HeadInfo[HEAD_INFO_LEN] = {0};

	CHECK_NULL_POINTER(__FUNCTION__, pEsFile);

	au8HeadInfo[0] = 0x01;

	//将长度u32FrmLen保存到第4-7字节
	au8HeadInfo[4] = (u32FrmLen >> 24 ) & 0xFF;
	au8HeadInfo[5] = (u32FrmLen >> 16 ) & 0xFF;
	au8HeadInfo[6] = (u32FrmLen >> 8  ) & 0xFF;
	au8HeadInfo[7] =  u32FrmLen & 0xFF;

	if(HEAD_INFO_LEN !=  fwrite(au8HeadInfo, 1, HEAD_INFO_LEN, pEsFile))
	{
		printf("Func:%s, Line:%d, fwrite HeadInfo failed!\n", __FUNCTION__, __LINE__);
		return FAILURE;
	}

	return SUCCESS;
}

static S32 WrtiteOneNalu(NALU_t *pstNalu, FILE 	*fpStream)
{
	CHECK_NULL_POINTER(__FUNCTION__, pstNalu);
	CHECK_NULL_POINTER(__FUNCTION__, fpStream);

	//每一个Nalu前面添加16字节的头信息
	if(SUCCESS != WrtiteHeadInfo(fpStream, pstNalu->len))
	{
		printf("Func:%s, Line:%d, WrtiteHeadInfo failed!\n", __FUNCTION__, __LINE__);
		return FAILURE;
	}

	if(1 !=  fwrite((void *)pstNalu->buf, pstNalu->len, 1, fpStream))
	{
		printf("Func:%s, Line:%d, fwrite file failed!\n", __FUNCTION__, __LINE__);
		return FAILURE;
	}

	return SUCCESS;
}

//判断是否为0x000001
static BOOL FindStartCode2(U8 *pu8Buf)
{
	CHECK_NULL_POINTER(__FUNCTION__, pu8Buf);

    if( (0 != pu8Buf[0]) || (0 != pu8Buf[1]) || (1 != pu8Buf[2]) )
    {
		return FALSE;
    }
    else
    {
		return TRUE; //0x000001
    }
}

//判断是否为0x00000001
static BOOL FindStartCode3 (U8 *pu8Buf)
{
	CHECK_NULL_POINTER(__FUNCTION__, pu8Buf);

    if( (0 != pu8Buf[0]) || (0 != pu8Buf[1]) || (0 != pu8Buf[2]) || (1 != pu8Buf[3]) )
    {
		return FALSE;
    }
    else
    {
		return TRUE; //0x00000001
    }
}

//按字节流查找NALU
S32 GetAnnexbNALU(NALU_t *pstNalu, FILE 	*fpStream)
{
    S32  s32Pos = 0;
	S32	 s32Rewind;
    U8   *pu8Buf = NULL;
    BOOL bStartCodeFound;

	CHECK_NULL_POINTER(__FUNCTION__, pstNalu);
	CHECK_NULL_POINTER(__FUNCTION__, fpStream);

    if (NULL == (pu8Buf = (U8 *)calloc(pstNalu->max_size, sizeof(char))))
    {
        printf("GetAnnexbNALU: calloc memory failed!\n");
		return FAILURE;
	}

    pstNalu->startcodeprefix_len = 3;

    if(3 != fread(pu8Buf, 1, 3, fpStream))
	{
        free(pu8Buf);
        return FAILURE;
    }

    bInfo2 = FindStartCode2(pu8Buf);
    if(TRUE != bInfo2)
	{
        if(1 != fread(pu8Buf + 3, 1, 1, fpStream))
		{
			free(pu8Buf);
			return FAILURE;
        }

        bInfo3 = FindStartCode3(pu8Buf);
        if (TRUE != bInfo3)
		{
			free(pu8Buf);
			return FAILURE;
        }
        else
		{
            s32Pos = 4;
            pstNalu->startcodeprefix_len = 4;
        }
    }
    else
	{
        pstNalu->startcodeprefix_len = 3;
        s32Pos = 3;
    }

    bStartCodeFound = FALSE;
    bInfo2 = FALSE;
    bInfo3 = FALSE;

    while(!bStartCodeFound)
	{
        if (feof(fpStream))
		{
            pstNalu->len = s32Pos - 1;
            memcpy(pstNalu->buf, pu8Buf, pstNalu->len);
			// 这里的赋值仅针对H264，由于后续未使用到，所以暂无意义，不影响H265码流的解析
            //pstNalu->forbidden_bit 		= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x80; // 1 bit
            //pstNalu->nal_reference_idc 	= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x60; // 2 bit
            //pstNalu->nal_unit_type 		= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x1f; // 5 bit

            free(pu8Buf);
            return s32Pos - 1;
        }

        pu8Buf[s32Pos++] = fgetc(fpStream);
        bInfo3 = FindStartCode3(&pu8Buf[s32Pos - 4]);
        if(TRUE != bInfo3)
        {
            bInfo2 = FindStartCode2(&pu8Buf[s32Pos - 3]);
        }
        bStartCodeFound = (bInfo2 == TRUE || bInfo3 == TRUE);
    }

    // 找到前后两个nalu的起始码，并且读取了起始码的长度导致文件指针的偏移，需要返回NALU的首地址
    s32Rewind = (bInfo3 == TRUE)? -4 : -3;

    if (0 != fseek (fpStream, s32Rewind, SEEK_CUR))
	{
        free(pu8Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file!\n");
    }

    // pstNalu->len表示除起始码之外的NALU的长度
    // s32Pos + s32Rewind 表示包含起始码的NALU的长度
    // 当前NALU的长度，由找到下一个NALU头来确定

	pstNalu->len = s32Pos + s32Rewind;
	memcpy (pstNalu->buf, pu8Buf, pstNalu->len);//
	//pstNalu->forbidden_bit		= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x80; // 1 bit
	//pstNalu->nal_reference_idc	= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x60; // 2 bit
	//pstNalu->nal_unit_type		= (pstNalu->buf[pstNalu->startcodeprefix_len]) & 0x1f; // 5 bit

    free(pu8Buf);

    return (s32Pos + s32Rewind);
}

S32 H26x_parser(S8 *ps8SrcFileName, S8 *ps8DstFileName)
{
    U32 	u32NalBufsize = MAX_NAIL_BUFF_SIZE;
    NALU_t  stNalu;

	FILE 	*fpSrcFile = NULL;
	FILE 	*fpDstFile = NULL;

	CHECK_NULL_POINTER(__FUNCTION__, ps8SrcFileName);
	CHECK_NULL_POINTER(__FUNCTION__, ps8DstFileName);

	memset(&stNalu, 0, sizeof(NALU_t));

    if(NULL == (fpSrcFile = fopen(ps8SrcFileName, "rb+")))
	{
        printf("fopen %s error\n", ps8SrcFileName);
        return FAILURE;
    }

    if(NULL == (fpDstFile = fopen(ps8DstFileName, "wb+")))
	{
        printf("fopen %s error\n", ps8DstFileName);
        return FAILURE;
    }

    stNalu.max_size = u32NalBufsize;
    if(NULL == (stNalu.buf = (S8 *)calloc(u32NalBufsize, sizeof(S8))))
	{
        free(stNalu.buf);
        printf("Alloc NALU stNalu.buf failed!\n");
        return FAILURE;
    }

    while(!feof(fpSrcFile))
    {
		if(FAILURE == GetAnnexbNALU(&stNalu, fpSrcFile))
		{
			printf("%s,%d, GetAnnexbNALU failed!\n", __FUNCTION__, __LINE__);
			return FAILURE;
		}

		if(SUCCESS !=  WrtiteOneNalu(&stNalu, fpDstFile))
		{
			printf("Func:%s, Line:%d, WrtiteOneNalu failed!\n", __FUNCTION__, __LINE__);
			return FAILURE;
		}
    }

    free(stNalu.buf);
    stNalu.buf = NULL;

    return SUCCESS;
}

S32 main(S32 argc, S8* argv[])
{
	S8 as8SrcFileName[MAX_FILE_NAME_LEN] = "";
	S8 as8DstFileName[MAX_FILE_NAME_LEN] = "";

	if(3 != argc)
	{
		printf("Input parameter err!!\n");
		printf("Usage: %s [SrcFile] [DstFile]\n", argv[0]);
		return FAILURE;
	}

	CHECK_NULL_POINTER(__FUNCTION__, argv[1]);
	CHECK_NULL_POINTER(__FUNCTION__, argv[2]);

	strncpy(as8SrcFileName, argv[1], MAX_FILE_NAME_LEN - 1);
	strncpy(as8DstFileName, argv[2], MAX_FILE_NAME_LEN - 1);

	if(SUCCESS != H26x_parser(as8SrcFileName, as8DstFileName))
	{
		printf("H26x_parser failed!!\n");
		return FAILURE;
	}

	return SUCCESS;
}







