#ifndef __VDEC_H__
#define __VDEC_H__

#include "sys.h"


typedef struct stVdecInfo_s
{
    int picWidth;
    int picHeight;
    int decodeType;
    int dpBufMode;
    int refFrameNum;
    unsigned int bitstreamSize;
}stVdecInfo_t;

typedef struct stDecOutInfo_s
{
    int intPortId;
    int intDecOutWidth;
    int intDecOutHeight;
}stDecOutInfo_t;

class Vdec: public Sys
{
public:
    Vdec();
    virtual ~Vdec();
    void GetInfo(stVdecInfo_t &info, std::vector<stDecOutInfo_t> &out)
    {
       info = stVdecInfo;
       out = vDecOutInfo;
    }
    void UpdateInfo(stVdecInfo_t &info, std::vector<stDecOutInfo_t> &out)
    {
        stVdecInfo = info;
        vDecOutInfo = out;
    };

private:
    virtual void LoadDb();
    virtual void Init();
    virtual void Deinit();
    stVdecInfo_t stVdecInfo;
    std::vector<stDecOutInfo_t> vDecOutInfo;
};
#endif

