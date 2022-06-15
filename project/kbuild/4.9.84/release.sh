#!/bin/bash

while getopts "k:i:a:b:p:f:c:l:v:" opt; do
  case $opt in
    k)
		kl_dir=$OPTARG
		;;
	a)
		arch=$OPTARG
		;;
	b)
		board=$OPTARG
		;;
	i)
		api_version=$OPTARG
		;;
	p)
		product=$OPTARG
		;;
	f)
		flash_type=$OPTARG
		;;
	c)
		chip=$OPTARG
		;;
	l)
		clib=$OPTARG
		;;
	v)
		toolchain_ver=$OPTARG
		;;
	\?)
	  echo "Invalid option: -$OPTARG" >&2
	  ;;
  esac
done

if [ "${chip}" = "i5" ]
then
	chip_name="infinity5"
fi
if [ "${chip}" = "i6" ]
then
	chip_name="infinity6"
fi
if [ "${chip}" = "i2m" ]
then
	chip_name="infinity2m"
fi
if [ "${chip}" = "i6e" ]
then
	chip_name="infinity6e"
fi

#kl_dir=$1
#api_version=$2

if [ "${arch}" = "" ]
then
	echo
	echo "  ARCH is not specified via -a parameter, set to arm !!"
	arch=arm
fi


ignore_kl_api_check=""
#if [ "${api_version}" = "" ]
#then
#	ignore_kl_api_check=1
#fi

if [ ! -e "${kl_dir}/arch/${arch}/boot/Image" ]
then
    echo "  File ${kl_dir}/arch/${arch}/boot/Image is missing , packing aborted!!"
    exit -1
fi

if [ -e "${kl_dir}/arch/${arch}/boot/xipImage" ]
then
    echo "  XIP will also be released as xipImage existed !!"
    XIP_RELEASE=1
fi

echo

copy_api=
RELEASEDIR=../../release/${product}/${chip}/${board}/${clib}/${toolchain_ver}/bin/kernel/${flash_type}
RELEASEDIR_MODULES=./${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/modules
TMP=.

kl_api_ver=$(cat ${kl_dir}/drivers/sstar/include/mdrv_API_version.h | grep 'KL_API_VERSION'| sed -n "s/.*\"\(.*\)\"/\1/p")
#if [ "${ignore_kl_api_check}" = "" ]
#then
#	if [ "${kl_api_ver}" != "${api_version}" ]
#	then
#		echo "  KL_API version ${api_version} is not matched ${kl_api_ver} from KL_DIR!!"
#		exit -1
#	fi
#else
#
#	echo "  Disable KL_API version check..."
#	echo "  Setting KL_API version to ${kl_api_ver} from KL_DIR"
#	api_version=${kl_api_ver}
#
#fi

kernel_id=$(strings ${kl_dir}/arch/${arch}/boot/Image | grep MVX.*KL_LX | cut -b 17-28 | sed -n 's/KL_\([^#]*\)#*/\1/p')
platform_id=$(strings ${kl_dir}/arch/${arch}/boot/Image | grep MVX.*KL_LX | cut -b 7-8)
commit=$(strings ${kl_dir}/arch/${arch}/boot/Image | grep MVX.*KL_LX | cut -b 10-16)
ori_commit=$(strings ${kl_dir}/arch/${arch}/boot/Image | grep MVX.*KL_LX | cut -b 10-16)
echo
echo "  BOARD = ${board}"
echo "  KL_DIR = ${kl_dir}"
echo "  KL_ARCH = ${arch}"
echo "  KL_API = ${kl_api_ver}"
echo "  KERNEL_ID = ${kernel_id}"
echo "  PLATFORM_ID = ${platform_id}"
echo "  COMMIT = ${commit}"
if [ "${XIP_RELEASE}" = "1" ]
then
  xip_commit=$(strings ${kl_dir}/arch/${arch}/boot/xipImage | grep MVX.*KL_LX | cut -b 10-16)
  echo "  xipImage COMMIT = ${xip_commit}"
fi
echo

echo "  Clean old object..."
rm -Rvf ${chip}/arch/${arch}/boot/
rm -Rvf ${chip}/drivers/sstar
rm -Rvf ${RELEASEDIR}/*Image*
rm -Rvf ./${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include
rm -Rvf ./${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/modules/*.ko
echo

mkdir -p ${chip}/drivers/
if [ ! -d "${RELEASEDIR}" ]
then
	echo  "mkdir $RELEASEDIR"
	mkdir -p "$RELEASEDIR" 
fi

if [ ! -d "${RELEASEDIR_MODULES}" ]
then
	echo  "mkdir $RELEASEDIR_MODULES"
	mkdir -p "$RELEASEDIR_MODULES" 
fi
echo

echo "  Preparing API..."
echo [${TMP}/${chip}/drivers/sstar/include]
mkdir -p ${TMP}/${chip}/drivers/sstar
cp -Rvf ${kl_dir}/drivers/sstar/include ${TMP}/${chip}/drivers/sstar
echo


#echo  "  Copy platform related DTS..."
#echo [${TMP}/${chip}/arch/${arch}/boot/dts]
#mkdir -p ${chip}/arch/${arch}/boot/ 
#mkdir -p ${chip}/arch/${arch}/boot/dts/
#cp -Rvf ${kl_dir}/arch/${arch}/boot/dts/${chip_name}* ${TMP}/${chip}/arch/${arch}/boot/dts/
##cp -Rvf ${kl_dir}/arch/${arch}/boot/dts/skeleton* ${TMP}/${chip}/arch/${arch}/boot/dts/
##mkdir -p ${TMP}/${chip}/arch/${arch}/boot/dts/include/dt-bindings
##cp -Rvf ${kl_dir}/arch/${arch}/boot/dts/include/dt-bindings/* ${TMP}/${chip}/arch/${arch}/boot/dts/include/dt-bindings/
#echo

echo [${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}]
cp -v ${kl_dir}/.config  ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}
cp -v ${kl_dir}/Module.symvers  ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}
mkdir ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include
mkdir ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include/config
cp -v ${kl_dir}/include/config/auto.conf ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include/config/
cp -Rvf ${kl_dir}/include/generated/ ${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include/

if [ "${clib}" = "glibc" ]
then
	sed -i "s/CONFIG_CROSS_COMPILE=\""/CONFIG_CROSS_COMPILE=\"arm-linux-gnueabihf-"/g" "${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include/config/auto.conf"
fi
if [ "${clib}" = "uclibc" ]
then
	sed -i "s/CONFIG_CROSS_COMPILE=\""/CONFIG_CROSS_COMPILE=\"arm-buildroot-linux-uclibcgnueabihf-"/g" "${TMP}/${chip}/configs/${product}/${board}/${clib}/${toolchain_ver}/${flash_type}/include/config/auto.conf"
fi
echo


#if [ "${ignore_kl_api_check}" = "" ]
#then
#	echo "  Local API directory existed. Checking API consistency..."
#	echo
#	if [ -e drivers/sstar/include ]; then
#		api_version=$(cat drivers/sstar/include/mdrv_API_version.h | grep 'KL_API_VERSION'| sed -n "s/.*\"\(.*\)\"/\1/p")
#	fi
#	res=$(diff -qur ${TMP}/${chip}/ ./ -x ms_version.h)
#	if [ "${res}" != "" ]
#	then
#		echo "Warning!! Current API with version ${api_version} is not consist with KL_API !!"
#                echo
#		echo "Reason: "
#		echo "${res}"
#                echo
#		echo "API will be updated!!"
#		copy_api=1
#	else
#		echo "  API check passed."
#	fi
#else
#	copy_api=1
#fi
#echo

echo "  Preparing Image..."
echo [${RELEASEDIR}]
#cp -f ${kl_dir}/arch/${arch}/boot/Image ${RELEASEDIR}/Image
#cp -f ${kl_dir}/arch/${arch}/boot/Image ${RELEASEDIR_NOR}/Image


if [ -e ${kl_dir}/arch/${arch}/boot/Image-zebu ]
then
	cp -vf ${kl_dir}/arch/${arch}/boot/Image-zebu ${RELEASEDIR}/Image-zebu
fi

if [ -e ${kl_dir}/arch/${arch}/boot/Image ]
then
	cp -vf ${kl_dir}/arch/${arch}/boot/Image ${RELEASEDIR}/Image
fi

if [ -e ${kl_dir}/arch/${arch}/boot/uImage.xz ]
then
	cp -vf ${kl_dir}/arch/${arch}/boot/uImage ${RELEASEDIR}/uImage
	cp -vf ${kl_dir}/arch/${arch}/boot/uImage ${RELEASEDIR_NOR}/uImage
	cp -vf ${kl_dir}/arch/${arch}/boot/uImage.xz ${RELEASEDIR}/uImage.xz
fi

if [ -e ${kl_dir}/arch/${arch}/boot/uImage.mz ]
then
	cp -vf ${kl_dir}/arch/${arch}/boot/uImage.mz ${RELEASEDIR}/uImage.mz
fi

if [ "${XIP_RELEASE}" = "1" ]
then
	cp -vf ${kl_dir}/arch/${arch}/boot/xipImage ${RELEASEDIR}/xipImage
fi
echo

echo "  Preparing Modules..."
echo [${RELEASEDIR_MODULES}]
cp -vf ${kl_dir}/modules/*.ko ${RELEASEDIR_MODULES}
echo

#if [ -e ${kl_dir}/modules ]
#then
#	cp -Rf ${kl_dir}/modules ${TMP}/${platform_id}/${kernel_id}/
#else
#	mkdir -p ${TMP}/${platform_id}/${kernel_id}/modules
#	exec < ${kl_dir}/modules.order
#	while read line
#	do
#	    module_file=$(echo ${line} | sed -e 's/^kernel\///g')
	    #echo ${module_file}
#	    file_name=$(echo ${module_file} | sed 's/.*\///g')
	    #echo f=${file_name}
#	    if [ -e "${TMP}/${platform_id}/${kernel_id}/modules/${file_name}" ]
#	    then
#		      echo "  ERROR!! Duplicated module file: ${module_file} "
#		      echo "  Please check module list to avoid this problem. "
#	          echo " Aborted"
#		      exit -1
#	    fi
#	    echo "    cp -f ${kl_dir}/${module_file} ${TMP}/${platform_id}/${kernel_id}/modules/${file_name}"
#	    cp -f ${kl_dir}/${module_file} ${TMP}/${platform_id}/${kernel_id}/modules/${file_name}
#	echo ${file_name} >> ${TMP}/${platform_id}/${kernel_id}/modules/modules.list
#	done
#fi
#if [ "${XIP_RELEASE}" = "1" ]
#then
#	cp -Rf ${kl_dir}/xipmodules ${TMP}/${platform_id}/${kernel_id}/
#fi

#echo "  Copying Files..."
#if [ "${copy_api}" != "" ]
#then
#	echo "    Updating API/..."
#	rm -Rf API
#	cp -Rf ${TMP}/API ./
#fi

#echo "    Updating Image & modules ..."
#rm -Rf ${platform_id}/${kernel_id}
#mkdir -p ${platform_id}/${kernel_id}
#cp -Rf  ${TMP}/${platform_id}/${kernel_id}/* ${platform_id}/${kernel_id}/
#echo

echo "  Removing Unecessary Files..."
rm -Rvf ${chip}/include/ms_version.h
rm -Rvf ${chip}/drivers/sstar/include/cedric/
rm -Rvf ${chip}/drivers/sstar/include/chicago/
rm -Rvf ${chip}/drivers/sstar/include/infinity/
rm -Rvf ${chip}/drivers/sstar/include/infinity3/
rm -Rvf ${chip}/drivers/sstar/include/infinity2/
echo

echo "  Remove white space & EOF..."
find ${chip}/drivers/sstar -type f \( -name '*.h' \) -exec sed -i 's/[[:space:]]*$//' '{}' \;
find ${chip}/arch/arm -type f \( -name '*.h' \) -exec sed -i 's/[[:space:]]*$//' '{}' \;
find ${chip}/arch/arm -type f \( -name '*.dts' \) -exec sed -i 's/[[:space:]]*$//' '{}' \;
find ${chip}/arch/arm -type f \( -name '*.dtsi' \) -exec sed -i 's/[[:space:]]*$//' '{}' \;
find ${chip}/drivers/sstar -type f -name '*.h' -exec sed --in-place -e :a -e '/^\n*$/{$d;N;};/\n$/ba' {} \;
find ${chip}/arch/arm -type f -name '*.h' -exec sed --in-place -e :a -e '/^\n*$/{$d;N;};/\n$/ba' {} \;
find ${chip}/arch/arm -type f -name '*.dtsi' -exec sed --in-place -e :a -e '/^\n*$/{$d;N;};/\n$/ba' {} \;
find ${chip}/arch/arm -type f -name '*.dts' -exec sed --in-place -e :a -e '/^\n*$/{$d;N;};/\n$/ba' {} \;
echo


echo "  IPCM/kernel release done!!"
echo "---------------------------------------------------------------"
echo "update kernel to "${commit}
echo "git log --pretty=format:\"%h %cd %an: %s\" --date=short "${ori_commit}.."${commit}"
echo "---------------------------------------------------------------"
