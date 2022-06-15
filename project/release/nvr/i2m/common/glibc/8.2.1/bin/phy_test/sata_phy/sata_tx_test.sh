#!/bin/sh
#
echo =============================
echo == SATA TX Test ver 0.01 ==
echo =============================
#-txmode : Tx test,
#         '-txmode <port no.> <gen no.> <tx mode> <SSC>', <port no.>: 0 or 1, <gen no.>: 1 ~ 3
#         <tx mode>: 1 :  HFTP, 2: MFTP, 3: LFTP, 4: LBP, 5: SSOP
#         <SSC>: 0 :  disable SSC, 1: enable SSC

echo ========== PORT 0 GEN 1 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 0 1 1 0
echo == MFTP without SSC ==
./sata_test -txmode 0 1 2 0
echo == LFTP without SSC ==
./sata_test -txmode 0 1 3 0
echo == LBP without SSC ==
./sata_test -txmode 0 1 4 0
echo == SSOP without SSC ==
./sata_test -txmode 0 1 5 0
echo == HFTP with SSC ==
./sata_test -txmode 0 1 1 1
echo == MFTP with SSC ==
./sata_test -txmode 0 1 2 1
echo == LFTP with SSC ==
./sata_test -txmode 0 1 3 1
echo == LBP with SSC ==
./sata_test -txmode 0 1 4 1
echo == SSOP with SSC ==
./sata_test -txmode 0 1 5 1
echo ========== PORT 0 GEN 1 test End ===========

echo ========== PORT 0 GEN 2 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 0 2 1 0
echo == MFTP without SSC ==
./sata_test -txmode 0 2 2 0
echo == LFTP without SSC ==
./sata_test -txmode 0 2 3 0
echo == LBP without SSC ==
./sata_test -txmode 0 2 4 0
echo == SSOP without SSC ==
./sata_test -txmode 0 2 5 0
echo == HFTP with SSC ==
./sata_test -txmode 0 2 1 1
echo == MFTP with SSC ==
./sata_test -txmode 0 2 2 1
echo == LFTP with SSC ==
./sata_test -txmode 0 2 3 1
echo == LBP with SSC ==
./sata_test -txmode 0 2 4 1
echo == SSOP with SSC ==
./sata_test -txmode 0 2 5 1
echo ========== PORT 0 GEN 2 test End ===========

echo ========== PORT 0 GEN 3 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 0 3 1 0
echo == MFTP without SSC ==
./sata_test -txmode 0 3 2 0
echo == LFTP without SSC ==
./sata_test -txmode 0 3 3 0
echo == LBP without SSC ==
./sata_test -txmode 0 3 4 0
echo == SSOP without SSC ==
./sata_test -txmode 0 3 5 0
echo == HFTP with SSC ==
./sata_test -txmode 0 3 1 1
echo == MFTP with SSC ==
./sata_test -txmode 0 3 2 1
echo == LFTP with SSC ==
./sata_test -txmode 0 3 3 1
echo == LBP with SSC ==
./sata_test -txmode 0 3 4 1
echo == SSOP with SSC ==
./sata_test -txmode 0 3 5 1
echo ========== PORT 0 GEN 3 test End ===========

echo ========== PORT 1 GEN 1 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 1 1 1 0
echo == MFTP without SSC ==
./sata_test -txmode 1 1 2 0
echo == LFTP without SSC ==
./sata_test -txmode 1 1 3 0
echo == LBP without SSC ==
./sata_test -txmode 1 1 4 0
echo == SSOP without SSC ==
./sata_test -txmode 1 1 5 0
echo == HFTP with SSC ==
./sata_test -txmode 1 1 1 1
echo == MFTP with SSC ==
./sata_test -txmode 1 1 2 1
echo == LFTP with SSC ==
./sata_test -txmode 1 1 3 1
echo == LBP with SSC ==
./sata_test -txmode 1 1 4 1
echo == SSOP with SSC ==
./sata_test -txmode 1 1 5 1
echo ========== PORT 1 GEN 1 test End ===========

echo ========== PORT 1 GEN 2 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 1 2 1 0
echo == MFTP without SSC ==
./sata_test -txmode 1 2 2 0
echo == LFTP without SSC ==
./sata_test -txmode 1 2 3 0
echo == LBP without SSC ==
./sata_test -txmode 1 2 4 0
echo == SSOP without SSC ==
./sata_test -txmode 1 2 5 0
echo == HFTP with SSC ==
./sata_test -txmode 1 2 1 1
echo == MFTP with SSC ==
./sata_test -txmode 1 2 2 1
echo == LFTP with SSC ==
./sata_test -txmode 1 2 3 1
echo == LBP with SSC ==
./sata_test -txmode 1 2 4 1
echo == SSOP with SSC ==
./sata_test -txmode 1 2 5 1
echo ========== PORT 1 GEN 2 test End ===========

echo ========== PORT 1 GEN 3 test Start  ===========
echo == HFTP without SSC ==
./sata_test -txmode 1 3 1 0
echo == MFTP without SSC ==
./sata_test -txmode 1 3 2 0
echo == LFTP without SSC ==
./sata_test -txmode 1 3 3 0
echo == LBP without SSC ==
./sata_test -txmode 1 3 4 0
echo == SSOP without SSC ==
./sata_test -txmode 1 3 5 0
echo == HFTP with SSC ==
./sata_test -txmode 1 3 1 1
echo == MFTP with SSC ==
./sata_test -txmode 1 3 2 1
echo == LFTP with SSC ==
./sata_test -txmode 1 3 3 1
echo == LBP with SSC ==
./sata_test -txmode 1 3 4 1
echo == SSOP with SSC ==
./sata_test -txmode 1 3 5 1
echo ========== PORT 1 GEN 3 test End ===========
