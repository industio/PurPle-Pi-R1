#!/bin/sh
cat gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1.* > gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1
cat gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1plus.* > gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1plus
cat gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/f951.* > gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/f951
cat gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/lto1.* > gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/lto1
cat gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gdb.* > gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gdb
chmod a+x gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1
chmod a+x gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/cc1plus
chmod a+x gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/f951
chmod a+x gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/libexec/gcc/arm-linux-gnueabihf/8.2.1/lto1
chmod a+x gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gdb
PWD=`pwd`
echo "export PATH=$PATH:${PWD}/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/bin" >> ~/.bashrc
source ~/.bashrc
check_ok=`which arm-linux-gnueabihf-gcc`
if [ "${check_ok}" == "" ]; then
	echo "install toolchain fail"
else
	echo "toolchain install on ${check_ok}"
fi
