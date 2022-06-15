#include <stdio.h>
#include "sys.h"
#include "rtsp.h"
#include "venc.h"
#include "vpe.h"
#include "vif.h"
#include "divp.h"
#include "dla.h"
#include "ui.h"
#include "iq.h"
#include "file.h"
#include "vdec.h"
#include "disp.h"
#include "vdisp.h"
#include "ai.h"
#include "ao.h"


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
#if INTERFACE_VDEC
            case E_SYS_MOD_VDEC:
            {
                SysChild<Vdec> Vdec(strKey);
            }
            break;
#endif
#if INTERFACE_DISP
            case E_SYS_MOD_DISP:
            {
                SysChild<Disp> Disp(strKey);
            }
            break;
#endif
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
#if INTERFACE_DIVP
            case E_SYS_MOD_DIVP:
            {
                SysChild<Divp> Divp(strKey);
            }
            break;
#endif
            case E_SYS_MOD_DLA:
            {
                SysChild<Dla> Dla(strKey);
            }
            break;
#if INTERFACE_RGN
            case E_SYS_MOD_UI:
            {
                SysChild<Ui> Fdfr(strKey);
            }
            break;
#endif
#if INTERFACE_IQSERVER
            case E_SYS_MOD_IQ:
            {
                SysChild<Iq> Iq(strKey);
            }
            break;
#endif
            case E_SYS_MOD_FILE:
            {
                SysChild<File> File(strKey);
            }
            break;
#if INTERFACE_VDISP
            case E_SYS_MOD_VDISP:
            {
                SysChild<Vdisp> Vdisp(strKey);
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
#if INTERFACE_AO
            case E_SYS_MOD_AO:
            {
                SysChild<Ao> Ao(strKey);
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
int main(int argc, char **argv)
{
    std::map<std::string, unsigned int> mapModId;
    std::map<std::string, Sys *> maskMap;
    std::vector<Sys *> objVect;
    char getC = 0;

    if (argc != 2)
    {
        printf("Usage: ./%s xxx_ini_path\n", argv[0]);

        return -1;
    }
    mapModId["RTSP"] = E_SYS_MOD_RTSP;
    mapModId["VENC"] = E_SYS_MOD_VENC;
    mapModId["VPE"] = E_SYS_MOD_VPE;
    mapModId["DIVP"] = E_SYS_MOD_DIVP;
    mapModId["DISP"] = E_SYS_MOD_DISP;
    mapModId["VDEC"] = E_SYS_MOD_VDEC;
    mapModId["VIF"] = E_SYS_MOD_VIF;
    mapModId["VDISP"] = E_SYS_MOD_VDISP;
    mapModId["LDC"] = E_SYS_MOD_LDC;
    mapModId["DLA"] = E_SYS_MOD_DLA;
    mapModId["UI"] = E_SYS_MOD_UI;
    mapModId["IQ"] = E_SYS_MOD_IQ;
    mapModId["FILE"] = E_SYS_MOD_FILE;
    mapModId["AI"] = E_SYS_MOD_AI;
    mapModId["AO"] = E_SYS_MOD_AO;
    Sys::InitSys(argv[1], mapModId);
    do
    {
        printf("Press 'q' to exit!\n");
        getC = getchar();
    }while (getC != 'q');
    Sys::DeinitSys();

    return 0;
}
