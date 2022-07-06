#!/bin/bash

while getopts "f:p:q:o:m:" opt; do
  case $opt in
    f)
      flashtype=$OPTARG
      ;;
    p)
      chip=$OPTARG
      ;;
    q)
      fastboot=$OPTARG
      ;;
    m)
      FLASH_SIZE=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

DATE=$(date +%m%d)
RELEASEDIR=`pwd`
export ARCH=arm
export PROJECT=2D06

if [ ! -f project/image/rootfs/rootfs.tar.gz ]; then
	cat project/image/rootfs/rootfs_route.tar.gz.* > project/image/rootfs/rootfs.tar.gz
fi

if [ "${FLASH_SIZE}" = "128" ]; then
	cp project/image/configs/i2m/spinand.ubifs.p2.partition.config_128M project/image/configs/i2m/spinand.ubifs.p2.partition.config -f
	cp project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config_128M project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config
elif [ "${FLASH_SIZE}" = "256" ]; then
	cp project/image/configs/i2m/spinand.ubifs.p2.partition.config_256M project/image/configs/i2m/spinand.ubifs.p2.partition.config -f
	cp project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config_256M project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config
elif [ "${FLASH_SIZE}" = "512" ]; then
	cp project/image/configs/i2m/spinand.ubifs.p2.partition.config_512M project/image/configs/i2m/spinand.ubifs.p2.partition.config -f
	cp project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config_512M project/image/configs/i2m/spinand.ramfs-squashfs.p2.partition.config
else
	echo "---------------------- pls declare flash size(-m) --------------------"
	exit 1
fi
# build uboot
cd ${RELEASEDIR}/boot
declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
if [ "${flashtype}" = "nor" ]; then
	make infinity2m_defconfig
fi

# build uboot
cd ${RELEASEDIR}/boot
declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
if [ "${flashtype}" = "nor" ]; then
	make infinity2m_defconfig
else
	make infinity2m_spinand_defconfig
fi
#make clean
make -j8

if [ "${flashtype}" = "nor" ]; then
	if [ -d ../project/board/i2m/boot/nor/uboot ]; then
		cp u-boot.xz.img.bin ../project/board/i2m/boot/nor/uboot
	fi
else
	if [ -d ../project/board/i2m/boot/spinand/uboot ]; then
		cp u-boot_spinand.xz.img.bin ../project/board/i2m/boot/spinand/uboot
	fi
fi

#build kernel
cd ${RELEASEDIR}/kernel
declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-"
if [ "${flashtype}" = "nor" ]; then
	if [ "${fastboot}" = "fastboot" ]; then
		make infinity2m_ssc011a_s01a_fastboot_defconfig
	else
		make infinity2m_ssc011a_s01a_minigui_defconfig
	fi
else
	if [ "${fastboot}" = "fastboot" ]; then
		make infinity2m_spinand_ssc011a_s01a_minigui_fastboot_doublenet_defconfig
	else
		make infinity2m_spinand_ssc011a_s01a_minigui_doublenet_defconfig
	fi
	
fi
#make clean
make -j16

#build project
cd ${RELEASEDIR}/project
if [ "${flashtype}" = "nor" ]; then
	if [ "${fastboot}" = "fastboot" ]; then
		echo test fastboot
		./setup_config.sh ./configs/nvr/i2m/8.2.1/nor.glibc-ramfs.011a.64
	else
		if [ "${chip}" = "ssd201" ]; then
			./setup_config.sh ./configs/nvr/i2m/8.2.1/nor.glibc-squashfs.011a.64
		fi
		if [ "${chip}" = "ssd202" ]; then
			./setup_config.sh ./configs/nvr/i2m/8.2.1/nor.glibc-squashfs.011a.128
		fi
	fi
else
	if [ "${fastboot}" = "fastboot" ]; then
		if [ "${chip}" = "ssd201" ]; then 
			./setup_config.sh ./configs/nvr/i2m/8.2.1/spinand.ram-glibc-squashfs.011a.64
		elif [ "${chip}" = "ssd202" ]; then	
			./setup_config.sh ./configs/nvr/i2m/8.2.1/spinand.ram-glibc-squashfs.011a.128
		fi
	else
		if [ "${chip}" = "ssd201" ]; then
			./setup_config.sh ./configs/nvr/i2m/8.2.1/spinand.glibc.011a.64
		fi
		if [ "${chip}" = "ssd202" ]; then
			./setup_config.sh ./configs/nvr/i2m/8.2.1/spinand.glibc.011a.128
		fi
	fi

fi

cd ${RELEASEDIR}/project/kbuild/4.9.84
if [ "${flashtype}" = "nor" ]; then
	if [ "${fastboot}" = "fastboot" ]; then
		./release.sh -k ${RELEASEDIR}/kernel -b 011A-fastboot -p nvr -f nor -c i2m -l glibc -v 8.2.1
	else
		./release.sh -k ${RELEASEDIR}/kernel -b 011A -p nvr -f nor -c i2m -l glibc -v 8.2.1
	fi
else
	if [ "${fastboot}" = "fastboot" ]; then
		echo fast release
		./release.sh -k ${RELEASEDIR}/kernel -b 011A-fastboot -p nvr -f spinand -c i2m -l glibc -v 8.2.1
	else
		./release.sh -k ${RELEASEDIR}/kernel -b 011A -p nvr -f spinand -c i2m -l glibc -v 8.2.1
	fi
fi

cd ${RELEASEDIR}/project
make clean;make image-nocheck

#install Image
cd ${RELEASEDIR}
rm -rf ${RELEASEDIR}/images
cp ${RELEASEDIR}/project/image/output/images . -rf
