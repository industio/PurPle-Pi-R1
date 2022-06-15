IMAGE_INSTALL_DIR:=$(OUTPUTDIR)
INIT_FILE=$(OUTPUTDIR)/rootfs/etc/init.sh
LATE_INIT_FILE=$(OUTPUTDIR)/customer/demo.sh
KBUILD_ROOT=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)
RELEASE_ROOT=$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)
LIB_DIR_PATH:=$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)

-include $(PROJ_ROOT)/../sdk/verify/application/app.mk
.PHONY: rootfs root app
rootfs:root app
root:
	cd rootfs; tar xf rootfs.tar.gz -C $(OUTPUTDIR)
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs
	## ramdisk/other use /linuxrc , ramfs use /init
	if [ "$(rootfs$(FSTYPE))" = "ramfs" ]; then \
		mv $(OUTPUTDIR)/rootfs/linuxrc $(OUTPUTDIR)/rootfs/init ; \
	fi;
	tar xf $(RELEASE_ROOT)/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib/

	mkdir -p $(OUTPUTDIR)/rootfs/sstar_configs
	mkdir -p  $(miservice$(RESOUCE))
	mkdir -p $(OUTPUTDIR)/customer

	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/ini/* $(OUTPUTDIR)/rootfs/sstar_configs ;\
	fi;
	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/* $(OUTPUTDIR)/rootfs/sstar_configs ; \
	else \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/fbdev.ini $(OUTPUTDIR)/rootfs/sstar_configs ; \
		cp -rf $(PROJ_ROOT)/board/ini/LCM $(OUTPUTDIR)/rootfs/sstar_configs ; \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/model/LCM_fastboot.ini $(OUTPUTDIR)/rootfs/sstar_configs/LCM.ini;	\
	fi;
	sed -i "s/FB_BUFFER_LEN = 4096/FB_BUFFER_LEN = 3072/g" $(OUTPUTDIR)/rootfs/sstar_configs/fbdev.ini
	sed -i "s/4096=4M/3072=3M/g" $(OUTPUTDIR)/rootfs/sstar_configs/fbdev.ini
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/mmap/$(MMAP) $(OUTPUTDIR)/rootfs/sstar_configs/mmap.ini
	cp -rvf $(RELEASE_ROOT)/bin/config_tool/* $(OUTPUTDIR)/rootfs/sstar_configs/
	cd $(OUTPUTDIR)/rootfs/sstar_configs; chmod +x config_tool; ln -sf config_tool dump_config; ln -sf config_tool dump_mmap
	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/pq  $(miservice$(RESOUCE))/ ; \
		find   $(miservice$(RESOUCE))/pq/ -type f ! -name "Main.bin" -type f ! -name "Main_Ex.bin" -type f ! -name "Bandwidth_RegTable.bin"| xargs rm -rf ; \
	fi;

	cp -rf $(PROJ_ROOT)/board/$(CHIP)/vdec_fw  $(miservice$(RESOUCE))/

	mkdir -p $(OUTPUTDIR)/rootfs/config
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc

	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION)
	mkdir -p  $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION)

	#### /etc/mdev.conf
	touch ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mice 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mouse.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo event.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo '$$DEVNAME=bus/usb/([0-9]+)/([0-9]+) 0:0 0660 =bus/usb/%1/%2' >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	#### create /etc/init.sh /config/demo.sh
	if [ -f $(INIT_FILE) ]; then \
		rm  $(INIT_FILE); \
	fi;
	touch $(INIT_FILE)
	chmod 755 $(INIT_FILE)

	if [ -f $(LATE_INIT_FILE) ]; then \
		rm  $(LATE_INIT_FILE); \
	fi;
	touch $(LATE_INIT_FILE)
	chmod 755 $(LATE_INIT_FILE)

	chmod 755 $(RELEASE_ROOT)/bin/debug/*
	mkdir -p  $(miservice$(RESOUCE))/lib

	mkdir -p  $(OUTPUTDIR)/appconfigs
	mkdir -p $(OUTPUTDIR)/rootfs/appconfigs


	#### profile edit
	echo export PATH=\$$PATH:/config >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export TERMINFO=/config/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	sed -i '/^mount.*/d' $(OUTPUTDIR)/rootfs/etc/profile
	echo mkdir -p /dev/pts >> ${OUTPUTDIR}/rootfs/etc/profile
	#echo mount -t sysfs none /sys >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mount -t tmpfs mdev /dev >> $(OUTPUTDIR)/rootfs/etc/profile
	#echo mount -t debugfs none /sys/kernel/debug/ >>  $(OUTPUTDIR)/rootfs/etc/profile
	echo mdev -s >> $(OUTPUTDIR)/rootfs/etc/profile

	chmod 755 $(RELEASE_ROOT)/bin/debug/*
	cp -rvf $(RELEASE_ROOT)/bin/debug/riu_*  $(miservice$(RESOUCE))

	#----------------------------------------------------------------------#
	#----------------the rule is first into ramdisk------------------------#
	#----------------the second is into partition of config or customer----#
	# creat insmod ko scrpit

	#------------------------kernel ko start.-----------------#
	if [ -f "$(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list" ]; then \
		cat $(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list |\
			sed 's#\(.*\).ko#insmod /lib/modules/$(KERNEL_VERSION)/\1.ko#' > $(INIT_FILE); \
		cat $(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list | \
			sed 's#\(.*\).ko\(.*\)#$(KBUILD_ROOT)/$(FLASH_TYPE)/modules/\1.ko#' |\
				xargs -i cp -rvf {} $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION); \
		echo -e "# kernel_mod_list\n" >> $(INIT_FILE); \
	fi;

	# only support 128M
	#echo \echo 3 \> \/proc\/sys\/vm\/drop_caches > $(OUTPUTDIR)/customer/demo.sh;

	if [ -f "$(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list_late" ]; then \
		cat $(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list_late |\
			sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(LATE_INIT_FILE); \
		cat $(KBUILD_ROOT)/$(FLASH_TYPE)/modules/kernel_mod_list_late | \
			sed 's#\(.*\).ko\(.*\)#$(KBUILD_ROOT)/$(FLASH_TYPE)/modules/\1.ko#' |\
				xargs -i cp -rvf {}  $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION); \
		echo -e "# kernel_mod_list\n" >> $(LATE_INIT_FILE); \
	fi;
	#-----------------------kernel ko end.-------------------#

	#-------------misc ko start.------------------------#
	if [ -f "$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list" ]; then \
		cat $(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list | \
			sed 's#\(.*\).ko#insmod /lib/modules/$(KERNEL_VERSION)/\1.ko#' >> $(INIT_FILE); \
		cat $(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list | \
			sed 's#\(.*\).ko\(.*\)#$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/\1.ko#' | \
				xargs -i cp -rvf {} $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION); \
		echo -e "# misc_mod_list\n" >> $(INIT_FILE); \
	fi;

	#-------------mi ko start.------------------------#
	if [ -f "$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/.mods_depend" ]; then \
		cat $(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/.mods_depend | \
			sed '2,20s#\(.*\)#insmod /lib/modules/$(KERNEL_VERSION)/\1.ko\nif [ $$? -eq 0 ]; then\n  busybox mknod /dev/\1 c $$major $$minor\n   let minor++\nfi\n\n#' >> $(INIT_FILE); \
		cat $(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/.mods_depend | \
			sed 's#\(.*\)#$(LIB_DIR_PATH)/modules/$(KERNEL_VERSION)/\1.ko#' | \
				xargs -i cp -rvf {} $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION); \
		echo "#mi module" >> $(INIT_FILE); \
	fi;
	# misc late 
	if [ -f "$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list_late" ]; then \
		cat $(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list_late | \
			sed 's#\(.*\).ko#insmod /lib/modules/$(KERNEL_VERSION)/\1.ko#' >> $(INIT_FILE); \
		cat $(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/misc_mod_list_late | \
			sed 's#\(.*\).ko\(.*\)#$(RELEASE_ROOT)/modules/$(KERNEL_VERSION)/\1.ko#' | \
				xargs -i cp -rvf {} $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION); \
		echo -e "# misc_mod_list_late\n" >> $(INIT_FILE); \
	fi;
	#-------------misc ko end.------------------------#


	#---------------dynamic so start.-------------------#
	cp -rvf $(RELEASE_ROOT)/mi_libs/dynamic/* $(OUTPUTDIR)/rootfs/lib;
	find $(OUTPUTDIR)/rootfs/lib/ -name "*.so*" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;\

	cp -rvf $(RELEASE_ROOT)/ex_libs/dynamic/*  $(miservice$(RESOUCE))/lib;
	find  $(miservice$(RESOUCE))/lib/ -name "*.so*" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;\
	#----------------dynamic so end.-------------------#
	sed -i 's/mi_common/insmod \/lib\/modules\/$(KERNEL_VERSION)\/mi_common.ko\nmajor=\`cat \/proc\/devices \| busybox awk "\\\\$$2==\\""mi"\\" {print \\\\$$1}"\\n`\nminor=0/g' $(INIT_FILE); \
	echo "#mi module" >> $(INIT_FILE);
	sed -i '/#mi module/a	major=`cat /proc/devices | busybox awk "\\\\$$2==\\""mi_poll"\\" {print \\\\$$1}"`\nbusybox mknod \/dev\/mi_poll c $$major 0' $(INIT_FILE); \

	if [ `find  $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION) -name "*.ko" | wc -l` -gt 0 ]; then \
		find  $(miservice$(RESOUCE))/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded; \
	fi
	if [ `find $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION) -name "*.ko" | wc -l` -gt 0 ]; then \
		find $(OUTPUTDIR)/rootfs/lib/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded; \
	fi
	if [ -e $(OUTPUTDIR)/rootfs/sstar_configs/config_tool ]; then \
		find $(OUTPUTDIR)/rootfs/sstar_configs/config_tool | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded; \
	fi

	echo mkdir -p /dev/pts >> $(OUTPUTDIR)/rootfs/etc/profile
	#echo mount -t devpts devpts /dev/pts >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "if [ -e /etc/init.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "    /etc/init.sh" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mdev -s >> $(INIT_FILE);

	if [ "$(BOARD)" = "011A-fastboot" ]; then \
		sed -i 's/mi_sys.ko/mi_sys.ko cmdQBufSize=128 logBufSize=0 default_config_path=\/sstar_configs/g' $(INIT_FILE); \
		sed -i 's/fbdev.ko/fbdev.ko default_config_path_file=\/sstar_configs\/fbdev.ini/g' $(INIT_FILE); \
	fi;


	#cp -rf $(PROJ_ROOT)/../sdk/verify/application/customer_zk/* $(OUTPUTDIR)/customer/
	#cp -rf $(PROJ_ROOT)/../sdk/verify/application/customer_zk/etc/* $(OUTPUTDIR)/rootfs/etc/
	echo -e $(foreach block, $(USR_MOUNT_BLOCKS), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG))\n") >> $(OUTPUTDIR)/rootfs/etc/profile

	echo export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:/customer\/lib:\/config\/lib >> $(OUTPUTDIR)/rootfs/etc/profile;

	echo "busybox telnetd&" >> $(OUTPUTDIR)/rootfs/etc/profile
	#echo \/customer\/bin\/zkgui \& >> $(OUTPUTDIR)/rootfs/etc/profile;
	echo sleep 8 >> $(OUTPUTDIR)/rootfs/etc/profile;
	echo /customer/demo.sh >> $(OUTPUTDIR)/rootfs/etc/profile;
	
	if [ $(interface_wlan) = "enable" ]; then \
		mkdir -p  $(miservice$(RESOUCE))/wifi ; \
		if [ $(FLASH_TYPE) = "spinand" ]; then \
			cp -rf $(LIB_DIR_PATH)/wifi/libs/ap/*   $(miservice$(RESOUCE))/wifi ; \
			cp -rf $(LIB_DIR_PATH)/wifi/bin/ap/*   $(miservice$(RESOUCE))/wifi ; \
		fi;	\
		find $(LIB_DIR_PATH)/wifi/bin/ -maxdepth 1 -type f -exec cp -P {}  $(miservice$(RESOUCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/bin/ -maxdepth 1 -type l -exec cp -P {}  $(miservice$(RESOUCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/libs/ -maxdepth 1 -type f -exec cp -P {}  $(miservice$(RESOUCE))/wifi \; ;\
		find $(LIB_DIR_PATH)/wifi/libs/ -maxdepth 1 -type l -exec cp -P {}  $(miservice$(RESOUCE))/wifi \; ;\
		cp -rf $(LIB_DIR_PATH)/wifi/modules/*   $(miservice$(RESOUCE))/wifi ; \
		cp -rf $(LIB_DIR_PATH)/wifi/configs/*   $(miservice$(RESOUCE))/wifi ; \
	fi;
	if [ "$(appconfigs$(RESOUCE))" != "" ]; then \
		if [ -f "$(miservice$(RESOUCE))/wifi/wpa_supplicant.conf" ]; then	\
			mv  $(miservice$(RESOUCE))/wifi/wpa_supplicant.conf $(appconfigs$(RESOUCE));	\
			cp $(OUTPUTDIR)/appconfigs/wpa_supplicant.conf $(appconfigs$(RESOUCE))/wpa_supplicant.conf_bak;	\
		fi;	\
	fi;
	
	### wifi relative files.

	if [ -f "$(miservice$(RESOUCE))/wifi/wpa_supplicant.conf" ]; then \
		mv  $(miservice$(RESOUCE))/wifi/wpa_supplicant.conf $(OUTPUTDIR)/appconfigs/;  \
		cp $(OUTPUTDIR)/appconfigs/wpa_supplicant.conf $(OUTPUTDIR)/appconfigs/wpa_supplicant.conf_bak;	\
	fi;

	mkdir -p $(OUTPUTDIR)/vendor
	mkdir -p $(OUTPUTDIR)/customer
	mkdir -p $(OUTPUTDIR)/rootfs/vendor
	mkdir -p $(OUTPUTDIR)/rootfs/customer
