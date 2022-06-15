#ifndef __DISP_H__
#define __DISP_H__

#include "sys.h"


typedef struct stDispInfo_s
{
    int intDisplayWidth;
    int intDisplayHeight;
    int intLinkType;
    int intBackGroundColor;
}stDispInfo_t;

class Disp: public Sys
{
    public:
        Disp();
        virtual ~Disp();
    private:
        virtual void Init();
        virtual void Deinit();
        stDispInfo_t stDispInfo;
};
#endif

