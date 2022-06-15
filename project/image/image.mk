FSIMAGE_LIST:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))),$(IMAGE_LIST))
TARGET_FSIMAGE:=$(foreach n,$(FSIMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)
TARGET_NOFSIMAGE:=$(foreach n,$(filter-out $(FSIMAGE_LIST), $(IMAGE_LIST)),$(n)_nofsimage)

BOOT_FSIMAGE_LIST:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(BOOT_IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))),$(BOOT_IMAGE_LIST))
BOOT_TARGET_FSIMAGE:=$(foreach n,$(BOOT_FSIMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_fsimage)
BOOT_TARGET_NOFSIMAGE:=$(foreach n,$(filter-out $(BOOT_FSIMAGE_LIST), $(BOOT_IMAGE_LIST)),$(n)_nofsimage)

images: $(TARGET_FSIMAGE) $(TARGET_NOFSIMAGE)

%_ext4_fsimage : ./build/make_ext4fs
	@echo [[$@]]
	./build/make_ext4fs -S ./build/file_contexts -l $($(patsubst %_ext4_fsimage,%,$@)$(PATSIZE)) -b 1024 $(IMAGEDIR)/$(patsubst %_ext4_fsimage,%,$@).img $($(patsubst %_ext4_fsimage,%,$@)$(RESOUCE))

#the system can't bootup while use mkquashfs_xz compress.
%_$(FLASH_TYPE)_squashfs_fsimage: ./build/mksquashfs_xz
	@echo [[$@]]
	./build/mksquashfs_xz $($(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_squashfs_fsimage,%,$@).sqfs -comp xz -all-root

%_$(FLASH_TYPE)_jffs2_fsimage: ./build/mkfs.jffs2
	@echo [[$@]]
	./build/mkfs.jffs2  $($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(PATSIZE)) --pad=$($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(PATSIZE)) --eraseblock=0x10000 -d $($(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@)$(RESOUCE)) -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_jffs2_fsimage,%,$@).jffs2

# ubifs default settings
#ifeq ($(UBI_MLC_TYPE), 0)
ERASE_BLOCK_SIZE0   :=0x20000
NAND_PAGE_SIZE0     :=0x800
UBI_LEB_SIZE0       :=126976
RESERVED_SIZE0      :=0
#else ifeq ($(UBI_MLC_TYPE), 1)
ERASE_BLOCK_SIZE1   :=0x40000
NAND_PAGE_SIZE1     :=0x1000
UBI_LEB_SIZE1       :=253952
RESERVED_SIZE1      :=4
#else ifeq ($(UBI_MLC_TYPE), 2)
ERASE_BLOCK_SIZE2   :=0x100000
NAND_PAGE_SIZE2     :=0x2000
UBI_LEB_SIZE2       :=1032192
RESERVED_SIZE2      :=0
#else
ERASE_BLOCK_SIZE    :=0x20000
NAND_PAGE_SIZE      :=0x800
UBI_LEB_SIZE        :=126976
RESERVED_SIZE       :=0
#endif

define makebin
if [ $(1) != "0" ]; then \
	dd if=/dev/zero bs=$(1) count=1 | tr '\000' '\377' > $(2)_tmp;	\
	dd if=$(3) of=$(2)_tmp bs=$(1) count=1 conv=notrunc seek=0;	\
	for Row in {1..$(4)};do	\
		dd if=$(2)_tmp of=$(2) bs=$(1) count=1 conv=notrunc oflag=append;	\
	done;	\
	rm -rf $(2)_tmp;	\
fi;
endef



cis_nofsimage:
	@echo [[$@]]
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/boot/$(FLASH_TYPE)/partition/* $(IMAGEDIR)/boot/
	$(PROJ_ROOT)/image/makefiletools/bin/pnigenerator -s 64 -p 2048 -b 64 -k 1024 -u 8 -l 0x20000 -t "$(cis$(PATSIZE))(CIS),$(cis$(BOOTTAB))" -y "$(cis$(SYSTAB))" -o $(IMAGEDIR)/boot/PARTINFO.pni
	dd if=/dev/zero bs=$(cis$(PGSIZE)) count=2 | tr '\000' '\377' > $(cis$(RESOUCE))
	dd if=$(IMAGEDIR)/boot/GCIS.bin of=$(cis$(RESOUCE)) bs=$(cis$(PGSIZE)) count=1 conv=notrunc seek=0
	dd if=$(IMAGEDIR)/boot/PARTINFO.pni of=$(cis$(RESOUCE)) bs=$(cis$(PGSIZE)) count=1 conv=notrunc seek=1
	cat $(IMAGEDIR)/boot/SPINANDINFO.sni >> $(cis$(RESOUCE))

ipl_nofsimage ipl_cust_nofsimage uboot_nofsimage:
	@echo [[$@]]
	cp -vf $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/boot/
	cp -vf $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)_s.bin
	#if [ "$(filter $@, $(TARGET_NOFSIMAGE))" != "" ]; then	\
		#$(call makebin,$(shell printf "%d" $($(patsubst %_nofsimage,%,$@)$(DATASIZE))),$(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)_s.bin,$($(patsubst %_nofsimage,%,$@)$(RESOUCE)),$($(patsubst %_nofsimage,%,$@)$(COPIES)))	\
	#fi;

iplx_uboot_nofsimage:
	@echo [[$@]]
	cat $(iplx$(RESOUCE)) $(uboot$(RESOUCE)) > $(IMAGEDIR)/boot/BOOT_PART.bin

mxp_nofsimage:
	@echo [[$@]]
	$(PROJ_ROOT)/image/makefiletools/bin/mxpgenerator $(mxp$(BOOTTAB)) $(mxp$(SYSTAB)) $(mxp$(RESOUCE))

misc_nofsimage:
	@echo [[$@]]
ifeq ($(DUAL_OS), on)
	mkdir -p $(IMAGEDIR)/misc
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/iqfile/$(SENSOR_TYPE)/* $(IMAGEDIR)/misc/
	cp -f $(IMAGEDIR)/misc/$(SENSOR_TYPE)_api.bin $(IMAGEDIR)/misc/$(ISP_API_COLORNIGHT)
	cp -f $(IMAGEDIR)/misc/$(SENSOR_TYPE)_api.bin $(IMAGEDIR)/misc/$(ISP_API_DAY)
	cp -f $(IMAGEDIR)/misc/$(SENSOR_TYPE)_api.bin $(IMAGEDIR)/misc/$(ISP_API_NIGHT)
	rm -f $(IMAGEDIR)/misc/$(SENSOR_TYPE)_api.bin
	mv $(IMAGEDIR)/misc/$(SENSOR_TYPE)_iqfile.bin $(IMAGEDIR)/misc/$(ISP_IQ)
ifneq ($(ISP_AWB),)
	mv $(IMAGEDIR)/misc/$(SENSOR_TYPE)_alsc_cali.data $(IMAGEDIR)/misc/$(ISP_AWB)
endif
ifneq ($(ISP_ALSC),)
	mv $(IMAGEDIR)/misc/$(SENSOR_TYPE)_awb_cali.data $(IMAGEDIR)/misc/$(ISP_ALSC)
endif
ifneq ($(DLA_FIRMWARE),)
	cp -rf $(PROJ_ROOT)/board/$(CHIP)/dla_file/* $(IMAGEDIR)/misc/
endif
endif
	$(PROJ_ROOT)/image/makefiletools/script/lwfs_pack.py $(IMAGEDIR)/misc/ $(IMAGEDIR)/misc.bin
	rm -rf $(IMAGEDIR)/misc/

boot_nofsimage: $(BOOT_TARGET_NOFSIMAGE) $(BOOT_TARGET_FSIMAGE)
	@echo [[$@]]
	dd if=/dev/zero of=$(boot$(RESOUCE)) bs=1 count=0

logo_nofsimage:
	@echo [[$@]]
	$(PROJ_ROOT)/image/makefiletools/bin/dispcfggen -c -o $(logo$(RESOUCE)) -p $(LOGO_ADDR) -s $(BOOTLOGO_BUFSIZE) -d $(DISP_OUT_NAME)
	$(PROJ_ROOT)/image/makefiletools/bin/logogen -a -i $(PROJ_ROOT)/board/ini/misc/$(BOOTLOGO_FILE) -o $(logo$(RESOUCE))
	$(PROJ_ROOT)/image/makefiletools/bin/logogen -a -i $(PROJ_ROOT)/board/ini/misc/upgrade.jpg -o $(logo$(RESOUCE))

%_$(FLASH_TYPE)_ramfs_fsimage:
	@echo [[$@]]
	cd $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION);./scripts/gen_initramfs_list.sh -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ramfs_fsimage,%,$@).ramfs -u 0 -g 0 $(OUTPUTDIR)/rootfs/

kernel_nofsimage:
	@echo [[$@]]
	cp -rvf $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)

%_$(FLASH_TYPE)_gz_fsimage:
	@echo [[$@]]
	cp -rvf $(OUTPUTDIR)/initramfs.gz $(IMAGEDIR)/

ramdisk_nofsimage:
	@echo [[$@]]
	@ $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/scripts/mkimage -A arm -O linux -C none -a 0 -e 0 -n "RAMDISK" -d $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)

rtos_nofsimage:
	@echo [[$@]]
	#@$(PREFIX)objcopy -O binary -R .bss -R .bss_sram $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)
	cp $($(patsubst %_nofsimage,%,$@)$(RESOUCE)) $(IMAGEDIR)/$(patsubst %_nofsimage,%,$@)
%_$(FLASH_TYPE)_ubifs_fsimage: ./build/mkfs.ubifs
	@echo [[$@]]
	./build/mkfs.ubifs -F -r $($(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@)$(RESOUCE)) -o $(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@).ubifs -m $(NAND_PAGE_SIZE$(UBI_MLC_TYPE)) -e $(UBI_LEB_SIZE$(UBI_MLC_TYPE)) -c `./build/calc_nand_mfs.sh $(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@) $(NAND_PAGE_SIZE$(UBI_MLC_TYPE)) $(ERASE_BLOCK_SIZE$(UBI_MLC_TYPE)) $(RESERVED_SIZE$(UBI_MLC_TYPE)) $($(patsubst %_$(FLASH_TYPE)_ubifs_fsimage,%,$@)$(PATSIZE))`
