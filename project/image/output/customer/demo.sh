insmod /config/modules/4.9.84/cifs.ko
insmod /config/modules/4.9.84/nls_utf8.ko
insmod /config/modules/4.9.84/grace.ko
insmod /config/modules/4.9.84/sunrpc.ko
insmod /config/modules/4.9.84/lockd.ko
insmod /config/modules/4.9.84/nfs.ko
insmod /config/modules/4.9.84/nfsv2.ko
insmod /config/modules/4.9.84/fat.ko
insmod /config/modules/4.9.84/msdos.ko
insmod /config/modules/4.9.84/vfat.ko
insmod /config/modules/4.9.84/ntfs.ko
insmod /config/modules/4.9.84/usb-common.ko
insmod /config/modules/4.9.84/usbcore.ko
insmod /config/modules/4.9.84/ehci-hcd.ko
insmod /config/modules/4.9.84/usb-storage.ko
insmod /config/modules/4.9.84/usbhid.ko
insmod /config/modules/4.9.84/mdrv_crypto.ko
#kernel_mod_list
insmod /config/modules/4.9.84/mhal.ko
#misc_mod_list
insmod /config/modules/4.9.84/mi_common.ko
major=`cat /proc/devices | busybox awk "\\$2==\""mi"\" {print \\$1}"\n`
minor=0
insmod /config/modules/4.9.84/mi_sys.ko cmdQBufSize=128 logBufSize=0
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_sys c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_gfx.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_gfx c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_divp.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_divp c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_vdec.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_vdec c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_ao.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_ao c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_disp.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_disp c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_ipu.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_ipu c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_ai.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_ai c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_venc.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_venc c $major $minor
	let minor++
fi


insmod /config/modules/4.9.84/mi_panel.ko
if [ $? -eq 0 ]; then
	busybox mknod /dev/mi_panel c $major $minor
	let minor++
fi


#mi module
major=`cat /proc/devices | busybox awk "\\$2==\""mi_poll"\" {print \\$1}"`
busybox mknod /dev/mi_poll c $major 0
insmod /config/modules/4.9.84/gpio_led_heartbeat.ko
insmod /config/modules/4.9.84/fbdev.ko
#misc_mod_list_late
mdev -s
export TERM=vt102
export TERMINFO=/config/terminfo
cd / 
./app_music &
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/customer/lib:/lib
cd /customer
chmod 777 disp_init
#echo 4 > /sys/class/gpio/export
#echo out > /sys/class/gpio/gpio4/direction
#echo 1 > /sys/class/gpio/gpio4/value
./disp_init &

