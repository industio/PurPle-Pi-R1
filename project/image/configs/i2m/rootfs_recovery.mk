.PHONY: rootfs_recovery

rootfs_recovery:
ifneq ($(rootfs_recovery$(RESOUCE)), )
	mkdir -p $(rootfs_recovery$(RESOUCE));
	cd rootfs; tar xf rootfs.tar.gz -C $(rootfs_recovery$(RESOUCE))
	mv -v $(rootfs_recovery$(RESOUCE))/rootfs/* $(rootfs_recovery$(RESOUCE))
	rm -rfv $(rootfs_recovery$(RESOUCE))/rootfs/
	tar xf busybox/$(BUSYBOX).tar.gz -C $(rootfs_recovery$(RESOUCE))
	tar xf $(LIB_DIR_PATH)/package/$(LIBC).tar.gz -C $(rootfs_recovery$(RESOUCE))/lib
	cp -rf etc/* $(rootfs_recovery$(RESOUCE))/etc

	cp -rvf $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/fw_printenv/* $(rootfs_recovery$(RESOUCE))/etc/
	echo "$(ENV_CFG)" > $(rootfs_recovery$(RESOUCE))/etc/fw_env.config
	if [ "$(ENV_CFG1)" != "" ]; then \
		echo "$(ENV_CFG1)" >> $(rootfs_recovery$(RESOUCE))/etc/fw_env.config ; \
	fi;
	cd $(rootfs_recovery$(RESOUCE))/etc/;ln -sf fw_printenv fw_setenv
	echo mkdir -p /var/lock >> $(rootfs_recovery$(RESOUCE))/etc/init.d/rcS

	echo "$(REBOOT_TO_RECOVERY)" > $(OUTPUTDIR)/rootfs/etc/reboot_to_recovery.sh
	chmod +x $(OUTPUTDIR)/rootfs/etc/reboot_to_recovery.sh
	echo "$(REBOOT_TO_NORMAL)" > $(rootfs_recovery$(RESOUCE))/etc/reboot_to_normal.sh
	chmod +x $(rootfs_recovery$(RESOUCE))/etc/reboot_to_normal.sh

ifneq ($(ota$(RESOUCE)), )
	mkdir -p $(ota$(RESOUCE));
	mkdir -p $(rootfs_recovery$(RESOUCE))/$(ota$(MOUNTTG))
	echo -e "mount -t $(ota$(FSTYPE)) $(ota$(MOUNTPT)) $(ota$(MOUNTTG))" >> $(rootfs_recovery$(RESOUCE))/etc/init.d/rcS
endif
endif
