#!/bin/bash
DIR_ROOT="`pwd`"
DIR_IMAGES=image/output/images
TARGET_BIN=$DIR_IMAGES/autoupgrade

TOOL_FOLDER=${BUILD_SDK_FOLDER}/tools/

#source  ${DIR_ROOT}/configs/current.configs

BUILD_SDK_FOLDER=$(cd "$(dirname "$0")";pwd)

TFTPDOWNLOADADDR=0x21000000
TFTPDOWNLOADADDR_PART_PNI=0x21800000
KERNELBOOTADDR=0x22000000
SPLIT_EACH_FILE_SIZE=0x2C00000

function split_ubifs_for_upgrade {

IMAGE_NAME=$1
#customer.ubifs
UBI_D_NAME=$2
#customer
echo split customer image
if [ -f "${DIR_IMAGES}/${IMAGE_NAME}" ]; then
	cp ${DIR_IMAGES}/scripts ${DIR_IMAGES}/scripts_bin -rf
	#echo "# <- this is for comment / total file size must be less than 4KB" > ${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es
	#echo ubi part UBI >> ${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es

	SIZE_IMAGE_UBIFS=`stat --format=%s  ${DIR_IMAGES}/${IMAGE_NAME}`
	#if [ $SIZE_IMAGE_UBIFS -gt 36700160 ]; then
	if [ $SIZE_IMAGE_UBIFS -gt 20971520 ]; then
		rm ${DIR_IMAGES}/scripts/[[customer.es
		echo "# <- this is for comment / total file size must be less than 4KB" > ${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es
		echo ubi part UBI >> ${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es
		split -b 20m ${DIR_IMAGES}/${IMAGE_NAME} -d -a1 ${DIR_IMAGES}/${IMAGE_NAME}.
		#split -b 35m ${DIR_IMAGES}/${IMAGE_NAME} -d -a1 ${DIR_IMAGES}/${IMAGE_NAME}.
		#SPLIT_EACH_FILE_SIZE=0x2300000 #20m=0x1400000 10m=0xA00000 5m=0x500000 
		SPLIT_EACH_FILE_SIZE=0x1400000 #20m=0x1400000 10m=0xA00000 5m=0x500000 
		for i in `ls ${DIR_IMAGES}|grep customer.ubifs.`; do
		if [[ "$i" = "${IMAGE_NAME}.0" ]]; then
			SIZE_IMAGE_UBIFS_16="0x`echo "obase=16;${SIZE_IMAGE_UBIFS}"|bc`"
			echo -e
			echo -e "$i"
			echo -e "  split -b 35m :${SPLIT_EACH_FILE_SIZE}"
			echo -e "  total size :${SIZE_IMAGE_UBIFS_16}"
		else
			SIZE_IMAGE_UBIFS_16=
		fi
		echo "  $i is ready!" 
		echo tftp ${TFTPDOWNLOADADDR} $i>>${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es
		echo ubi write.part ${TFTPDOWNLOADADDR} ${UBI_D_NAME} ${SPLIT_EACH_FILE_SIZE} ${SIZE_IMAGE_UBIFS_16}>>${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es

	done
		echo "% <- this is end of file symbol" >>  ${DIR_IMAGES}/scripts/[[${UBI_D_NAME}.es
	else
		echo "${UBI_D_NAME}.es is not over size,do nothing!!"
	fi

	

fi
}

split_ubifs_for_upgrade customer.ubifs customer
#rm ${DIR_IMAGES}/scripts/[[data.ds
