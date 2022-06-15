DISP_TIMING_TYPE=${1:-LCD}
DISP_TIMING_LCD_HEADER=${2:-"SAT070CP50_1024x600.h"}

ENABLE_PNG=${3:-"yes"}
ENABLE_BMP=${4:-"no"}
ENABLE_JPEG=${5:-"yes"}
ENABLE_RAW=${6:-"yes"}
ENABLE_DEBUG=${7:-"yes"}
./configure CFLAGS="-rdynamic -funwind-tables" --host=arm-linux-gnueabihf --with-timingtype=${DISP_TIMING_TYPE} --with-lcdheader=${DISP_TIMING_LCD_HEADER}  ${@:3} --enable-png=${ENABLE_PNG} --enable-bmp=${ENABLE_BMP} --enable-jpeg=${ENABLE_JPEG} --enable-raw=${ENABLE_RAW} --enable-logodebug=${ENABLE_DEBUG}
