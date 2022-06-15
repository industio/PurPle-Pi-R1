/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.
  
  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (??Sigmastar Confidential Information??) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "WavFileWrite.h"

typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
}WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    signed char chRIFF[4];
    unsigned int dwRIFFLen;
    signed char chWAVE[4];
    signed char chFMT[4];
    unsigned int dwFMTLen;
    WaveFormat_t wave;
    signed char chDATA[4];
    unsigned int dwDATALen;
}WaveFileHeader_t;

typedef struct WAVEWRITEHANDLE
{
    WaveFileHeader_t header;
	FILE* fp_wav;
	int sample_rate;
	int channels;
	int bit_wide;
	MediaType_e type;
	int iRaw_len;
}WaveWriteHandle;

static void write_wave_header(WaveFileHeader_t* tWavHead, MediaType_e playload, int sample_rate, int channel, int bitWidth, signed int raw_len);
int wav_open(void** pHandle, const char *fileName, int sample_rate, int channels, int bit_wide, MediaType_e type) 
{
	*pHandle = NULL;
	WaveWriteHandle* p_pHandle = NULL;
	FILE* fp_wav = NULL;
	if ( fileName==NULL ) {
		return -1;
	} else if ( (fp_wav = fopen(fileName, "wb"))==NULL ) {
		return -2;
	}
	p_pHandle = (WaveWriteHandle *)malloc(sizeof(WaveWriteHandle));
	memset(p_pHandle, 0, sizeof(WaveWriteHandle));
	p_pHandle->fp_wav = fp_wav;
	p_pHandle->sample_rate = sample_rate;
	p_pHandle->channels    = channels;
	p_pHandle->bit_wide    = bit_wide;
	p_pHandle->type        = type;
	p_pHandle->iRaw_len    = 0;
	write_wave_header(&p_pHandle->header, type, sample_rate, channels, bit_wide, 0);
	fwrite(&p_pHandle->header, 1, sizeof(p_pHandle->header), p_pHandle->fp_wav);
	*pHandle = (void *)p_pHandle;
	return 0;
}

int wav_write(void *handle, char* data, int iLen)
{
	WaveWriteHandle* p_pHandle = (WaveWriteHandle*)handle;
	if ( handle==NULL ) {
		return -1;
	}
	int iRet = fwrite(data, 1, iLen, p_pHandle->fp_wav);
	p_pHandle->iRaw_len += iLen;
	return iRet;
}

int wav_close(void** pHandle)
{
	WaveWriteHandle* p_pHandle = (WaveWriteHandle *)(*pHandle);
	if ( pHandle==NULL ) {
		return -1;
	} else if ( p_pHandle==NULL ) {
		return -2;
	}
	write_wave_header(&p_pHandle->header, p_pHandle->type, p_pHandle->sample_rate, p_pHandle->channels, p_pHandle->bit_wide, p_pHandle->iRaw_len);
	fseek(p_pHandle->fp_wav, 0, SEEK_SET);
	fwrite(&p_pHandle->header, 1, sizeof(p_pHandle->header), p_pHandle->fp_wav);
	fclose(p_pHandle->fp_wav);
	p_pHandle->fp_wav = NULL;
	free(p_pHandle);
	*pHandle = NULL;
	return 0;
}

int wav_open_fseek(void *handle, long Offset, int Origin)
{
	WaveWriteHandle* p_pHandle = (WaveWriteHandle*)handle;
	if ( handle==NULL ) {
		return -1;
	}
	return fseek(p_pHandle->fp_wav, Offset, Origin);
}

int wav_read_head(FILE* fp_wav, MediaType_e* playload, int* iSamples, int* iChannel, int* iBitWide)
{
	if (fp_wav==NULL) {
		return -1;
	}
	long loldFtell=ftell(fp_wav);
	fseek(fp_wav, 0, SEEK_SET);
	char head[100]="";
	int iRet = fread(head, 1, 100, fp_wav);
	if ( iRet<=0 ) {
		return -2;
	}
	WaveFileHeader_t* pWaveFileHeader_t=(WaveFileHeader_t*)head;
	*iSamples = pWaveFileHeader_t->wave.dwSamplesPerSec;
	*iChannel = pWaveFileHeader_t->wave.wChannels;
	*iBitWide = pWaveFileHeader_t->wave.wBitsPerSample;
	*playload = (MediaType_e)pWaveFileHeader_t->wave.wFormatTag;
	fseek(fp_wav, loldFtell, SEEK_SET);
	return 0;
}



static void write_wave_header(WaveFileHeader_t* tWavHead, MediaType_e playload, int sample_rate, int channel, int bitWidth, signed int raw_len)
{
    tWavHead->chRIFF[0]='R';
    tWavHead->chRIFF[1]='I';
    tWavHead->chRIFF[2]='F';
    tWavHead->chRIFF[3]='F';

    tWavHead->chWAVE[0]='W';
    tWavHead->chWAVE[1]='A';
    tWavHead->chWAVE[2]='V';
    tWavHead->chWAVE[3]='E';

    tWavHead->chFMT[0]='f';
    tWavHead->chFMT[1]='m';
    tWavHead->chFMT[2]='t';
    tWavHead->chFMT[3]=' ';
    tWavHead->dwFMTLen = 0x10; // 0x10：不携带附加数据，0x12：携带附加数据
	
	tWavHead->wave.wChannels = channel;
	tWavHead->wave.dwSamplesPerSec = sample_rate;

	if(playload == MT_PCM) {
        tWavHead->wave.wBitsPerSample = bitWidth;
		tWavHead->wave.wBlockAlign =  (tWavHead->wave.wBitsPerSample  * tWavHead->wave.wChannels) / 8;
	} else if(playload==MT_G711U||playload==MT_G711A) {
        tWavHead->wave.wBitsPerSample = 8;//bitWidth;g711encode出来是8bit，这里需要写死
        tWavHead->wave.wBlockAlign =  (tWavHead->wave.wBitsPerSample  * tWavHead->wave.wChannels) / 8;
    } else if(playload==MT_G726) {
        tWavHead->wave.wChannels  = 0x01;
        tWavHead->wave.wBitsPerSample = 4;
        tWavHead->wave.dwSamplesPerSec = 8000;
        tWavHead->wave.wBlockAlign =  1;
    } else if(playload == MT_ADPCM) {
		tWavHead->wave.wBitsPerSample = 4;
		tWavHead->wave.wBlockAlign = 1024 ;
	}

	tWavHead->wave.wFormatTag = playload;
	tWavHead->wave.dwAvgBytesPerSec = (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels)/8;
    tWavHead->chDATA[0]='d';
    tWavHead->chDATA[1]='a';
    tWavHead->chDATA[2]='t';
    tWavHead->chDATA[3]='a';
    tWavHead->dwDATALen = raw_len;
    tWavHead->dwRIFFLen = raw_len+sizeof(WaveFileHeader_t) - 8;

	return ;
}