#!/bin/sh
#
echo =============================
echo == SATA unit test ver 0.01 ==
echo =============================
#-loopback       : loopback test,
#         '-loopback <port no.> <gen no.>', <port no.>: 0 or 1, <gen no.>: 1 ~ 3

echo ========== PORT 0 GEN 1 test Start  ===========
./sata_test -loopback 0 1
echo ========== PORT 0 GEN 1 test End ===========

echo ========== PORT 0 GEN 2 test Start  ===========
./sata_test -loopback 0 2
echo ========== PORT 0 GEN 2 test End ===========

echo ========== PORT 0 GEN 3 test Start  ===========
./sata_test -loopback 0 3
echo ========== PORT 0 GEN 3 test End ===========

echo ========== PORT 1 GEN 1 test Start  ===========
./sata_test -loopback 1 1
echo ========== PORT 1 GEN 1 test End ===========

echo ========== PORT 1 GEN 2 test Start  ===========
./sata_test -loopback 1 2
echo ========== PORT 1 GEN 2 test End ===========

echo ========== PORT 1 GEN 3 test Start  ===========
./sata_test -loopback 1 3
echo ========== PORT 1 GEN 3 test End ===========
