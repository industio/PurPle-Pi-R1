TFTPDOWNLOADADDR:=0x21000000
TFTPDOWNLOADADDR_PART_PNI:=0x21800000
KERNELBOOTADDR:=0x22000000

TARGET_SCRIPT:=$(foreach n,$(IMAGE_LIST),$(n)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_script) $(PAT_TABLE)_$(FLASH_TYPE)_partition_script $(PAT_TABLE)_$(FLASH_TYPE)_config_script
TARGET_FS:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))), $(IMAGE_LIST))
TARGET_UBIFS := $(patsubst %_fs_ubifs_, %, $(filter %_fs_ubifs_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_SQUAFS := $(patsubst %_fs_squashfs_, %,$(filter %_fs_squashfs_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_JIFFS2 := $(patsubst %_fs_jffs2_, %, $(filter %_fs_jffs2_, $(foreach n,$(TARGET_FS),$(n)_fs_$($(n)$(FSTYPE))_)))
TARGET_NONEFS := $(filter-out $(TARGET_FS), $(filter-out $(patsubst %_fs__sz__, %, $(filter %_fs__sz__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_sz_$($(n)$(PATSIZE))_))), $(IMAGE_LIST)))
TARGET_IMAGE_LIST := $(filter-out cis, $(IMAGE_LIST))
SCRIPTDIR:=$(IMAGEDIR)/scripts

scripts:
	mkdir -p $(SCRIPTDIR) 
	$(MAKE) set_partition
	$(MAKE) $(TARGET_SCRIPT)
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(IMAGEDIR)/auto_update.txt
	if [ "$(filter cis, $(IMAGE_LIST))" != "" ]; then	\
		echo estar scripts/[[cis.es >> $(IMAGEDIR)/auto_update.txt;	\
	fi;
	@echo estar scripts/[[set_partition.es >> $(IMAGEDIR)/auto_update.txt
	@echo -e $(foreach n,$(TARGET_IMAGE_LIST),estar scripts/[[$(n)\.es\\n) >> $(IMAGEDIR)/auto_update.txt
	@echo estar scripts/set_config >> $(IMAGEDIR)/auto_update.txt
	@echo saveenv >> $(IMAGEDIR)/auto_update.txt
	@echo printenv >> $(IMAGEDIR)/auto_update.txt
	@echo reset >> $(IMAGEDIR)/auto_update.txt
	@echo "% <- this is end of file symbol" >> $(IMAGEDIR)/auto_update.txt
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(IMAGEDIR)/auto_update_bin.txt
	if [ "$(filter cis, $(IMAGE_LIST))" != "" ]; then	\
		echo estar scripts_bin/[[cis.es >> $(IMAGEDIR)/auto_update_bin.txt;	\
	fi;
	@echo estar scripts_bin/[[set_partition.es >> $(IMAGEDIR)/auto_update_bin.txt
	@echo -e $(foreach n,$(TARGET_IMAGE_LIST),estar scripts_bin/[[$(n)\.es\\n) >> $(IMAGEDIR)/auto_update_bin.txt
	@echo estar scripts_bin/set_config >> $(IMAGEDIR)/auto_update_bin.txt
	@echo saveenv >> $(IMAGEDIR)/auto_update_bin.txt
	@echo printenv >> $(IMAGEDIR)/auto_update_bin.txt
	@echo reset >> $(IMAGEDIR)/auto_update_bin.txt
	@echo "% <- this is end of file symbol" >> $(IMAGEDIR)/auto_update_bin.txt

set_partition:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[set_partition.es
ifneq ($(MTDPARTS), )
	@echo setenv mtdparts \' $(MTDPARTS) >> $(SCRIPTDIR)/[[set_partition.es
else
	@echo mtdparts del CIS >> $(SCRIPTDIR)/[[set_partition.es
	@echo setenv mtdparts "\$$(mtdparts),$(cis$(SYSTAB))" >> $(SCRIPTDIR)/[[set_partition.es
endif
	@echo saveenv >> $(SCRIPTDIR)/[[set_partition.es
	@echo nand erase.part UBI >> $(SCRIPTDIR)/[[set_partition.es
	@echo ubi part UBI >> $(SCRIPTDIR)/[[set_partition.es
	@echo -e $(foreach n,$(TARGET_UBIFS),ubi create $(n) $($(n)$(PATSIZE))\\n) >> $(SCRIPTDIR)/[[set_partition.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[set_partition.es

cis_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[cis.es
	@echo tftp $(TFTPDOWNLOADADDR) boot/SPINANDINFO.sni >> $(SCRIPTDIR)/[[cis.es
	@echo tftp $(TFTPDOWNLOADADDR_PART_PNI) boot/PARTINFO.pni >> $(SCRIPTDIR)/[[cis.es
	@echo writecis $(TFTPDOWNLOADADDR) $(TFTPDOWNLOADADDR_PART_PNI) $(CIS_PBAs) $(CIS_COPIES) >> $(SCRIPTDIR)/[[cis.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[cis.es

ipl_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[ipl.es
	@echo tftp $(TFTPDOWNLOADADDR) ipl_s.bin >> $(SCRIPTDIR)/[[ipl.es
	@echo nand erase.part IPL0 >> $(SCRIPTDIR)/[[ipl.es
	@echo nand write.e $(TFTPDOWNLOADADDR) IPL0 \$${filesize} >> $(SCRIPTDIR)/[[ipl.es
	@echo nand erase.part IPL1 >> $(SCRIPTDIR)/[[ipl.es
	@echo nand write.e $(TFTPDOWNLOADADDR) IPL1 \$${filesize} >> $(SCRIPTDIR)/[[ipl.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[ipl.es

ipl_cust_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[ipl_cust.es
	@echo tftp $(TFTPDOWNLOADADDR) ipl_cust_s.bin > $(SCRIPTDIR)/[[ipl_cust.es
	@echo nand erase.part IPL_CUST0 >> $(SCRIPTDIR)/[[ipl_cust.es
	@echo nand write.e $(TFTPDOWNLOADADDR) IPL_CUST0 \$${filesize} >> $(SCRIPTDIR)/[[ipl_cust.es
	@echo nand erase.part IPL_CUST1 >> $(SCRIPTDIR)/[[ipl_cust.es
	@echo nand write.e $(TFTPDOWNLOADADDR) IPL_CUST1 \$${filesize} >> $(SCRIPTDIR)/[[ipl_cust.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[ipl_cust.es

kernel_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[kernel.es
	@echo tftp $(TFTPDOWNLOADADDR) kernel >> $(SCRIPTDIR)/[[kernel.es
	@echo nand erase.part KERNEL >> $(SCRIPTDIR)/[[kernel.es
	@echo nand write.e $(TFTPDOWNLOADADDR) KERNEL \$${filesize} >> $(SCRIPTDIR)/[[kernel.es
	@echo nand erase.part RECOVERY >> $(SCRIPTDIR)/[[kernel.es
	@echo nand write.e $(TFTPDOWNLOADADDR) RECOVERY \$${filesize} >> $(SCRIPTDIR)/[[kernel.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[kernel.es
	@echo kernel-image done!!!

uboot_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[uboot.es
	@echo tftp $(TFTPDOWNLOADADDR) uboot_s.bin >> $(SCRIPTDIR)/[[uboot.es
	@echo nand erase.part UBOOT0 >> $(SCRIPTDIR)/[[uboot.es
	@echo nand write.e $(TFTPDOWNLOADADDR) UBOOT0 \$${filesize} >> $(SCRIPTDIR)/[[uboot.es
	@echo nand erase.part UBOOT1 >> $(SCRIPTDIR)/[[uboot.es
	@echo nand write.e $(TFTPDOWNLOADADDR) UBOOT1 \$${filesize} >> $(SCRIPTDIR)/[[uboot.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[uboot.es

boot_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[boot.es
	@echo tftp $(TFTPDOWNLOADADDR) boot.bin >> $(SCRIPTDIR)/[[boot.es
	@echo nand erase.part BOOT >> $(SCRIPTDIR)/[[boot.es
	@echo nand write.e $(TFTPDOWNLOADADDR) BOOT \$${filesize} >> $(SCRIPTDIR)/[[boot.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[boot.es

logo_$(FLASH_TYPE)__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[logo.es
	@echo  tftp $(TFTPDOWNLOADADDR) logo >> $(SCRIPTDIR)/[[logo.es
	@echo nand erase.part LOGO >> $(SCRIPTDIR)/[[logo.es
	@echo nand write.e $(TFTPDOWNLOADADDR) LOGO \$${filesize} >> $(SCRIPTDIR)/[[logo.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[logo.es

%_$(FLASH_TYPE)_squashfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).es
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).sqfs >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).es
	@echo nand erase.part $(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).es
	@echo nand write.e $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@) \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_squashfs_script,%,$@).es

%_$(FLASH_TYPE)_ramfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[rootfs.es
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@).ramfs >> $(SCRIPTDIR)/[[rootfs.es
	@echo nand erase.part $(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@) >> $(SCRIPTDIR)/[[rootfs.es
	@echo nand write.e $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ramfs_script,%,$@) \$${filesize} >> $(SCRIPTDIR)/[[rootfs.es
	@echo setenv initrd_high ${INITRAMFSLOADADDR} >> $(SCRIPTDIR)/[[rootfs.es
	@echo setenv initrd_size \$${filesize} >> $(SCRIPTDIR)/[[rootfs.es
	@echo setenv initrd_block 77 >> $(SCRIPTDIR)/[[rootfs.es
	@echo nand erase.part rootfs_bak >> $(SCRIPTDIR)/[[rootfs.es
	@echo nand write.e $(TFTPDOWNLOADADDR) rootfs_bak \$${filesize} >> $(SCRIPTDIR)/[[rootfs.es
	@echo setenv initrd_bak_block 129 >> $(SCRIPTDIR)/[[rootfs.es
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[rootfs.es


%_$(FLASH_TYPE)_ubifs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).es
	@echo ubi part UBI >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).es
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).ubifs >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).es
	@echo ubi write $(TFTPDOWNLOADADDR) $(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@) \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).es
	@echo "% <- this is end of file symbol" >>  $(SCRIPTDIR)/[[$(patsubst %_$(FLASH_TYPE)_ubifs_script,%,$@).es

ubi_$(FLASH_TYPE)_partition_script:

ubi_$(FLASH_TYPE)_config_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/set_config
	@echo setenv bootargs  $(rootfs$(BOOTENV)) $(kernel$(BOOTENV)) $(EXBOOTARGS) \$$\(mtdparts\) >> $(SCRIPTDIR)/set_config
ifeq ($(PROJECT), 2D06)
	@echo setenv bootcmd \' bootlogo 0 0 0 0 0\; $(wifi24mclkcmd)\; $(wifirstoffcmd)\; nand read.e $(KERNELBOOTADDR) KERNEL $(kernel$(PATSIZE))\;$(rootfs$(BOOTCMD)) $(wifirstoncmd)\; bootm $(KERNELBOOTADDR)\;nand read.e $(KERNELBOOTADDR) RECOVERY $(kernel$(PATSIZE))\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/set_config
endif
ifeq ($(PROJECT), 2D07)
	@echo setenv bootcmd \'gpio output 18 1\; bootlogo 0 0 0 0 0\; $(wifi24mclkcmd)\; $(wifirstoffcmd)\; nand read.e $(KERNELBOOTADDR) KERNEL $(kernel$(PATSIZE))\;$(rootfs$(BOOTCMD)) $(wifirstoncmd)\; bootm $(KERNELBOOTADDR)\;nand read.e $(KERNELBOOTADDR) RECOVERY $(kernel$(PATSIZE))\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/set_config
endif
ifeq ($(PROJECT), xiangfan)
	@echo setenv bootcmd \'gpio output 4 1\; bootlogo 0 0 0 0 0\; $(wifi24mclkcmd)\; $(wifirstoffcmd)\; nand read.e $(KERNELBOOTADDR) KERNEL $(kernel$(PATSIZE))\;$(rootfs$(BOOTCMD)) $(wifirstoncmd)\; bootm $(KERNELBOOTADDR)\;nand read.e $(KERNELBOOTADDR) RECOVERY $(kernel$(PATSIZE))\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/set_config
else
	@echo setenv bootcmd \' bootlogo 0 0 0 0 0\; $(wifi24mclkcmd)\; $(wifirstoffcmd)\; nand read.e $(KERNELBOOTADDR) KERNEL $(kernel$(PATSIZE))\;$(rootfs$(BOOTCMD)) $(wifirstoncmd)\; bootm $(KERNELBOOTADDR)\;nand read.e $(KERNELBOOTADDR) RECOVERY $(kernel$(PATSIZE))\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/set_config
endif
	@echo setenv dispout $(DISP_OUT_NAME) >> $(SCRIPTDIR)/set_config
	@echo setenv sdautoupgrade 1 >> $(SCRIPTDIR)/set_config
	@echo setenv usbautoupgrade 1 >> $(SCRIPTDIR)/set_config
	@echo setenv UpgradePort 1 >> $(SCRIPTDIR)/set_config
	@echo saveenv >> $(SCRIPTDIR)/set_config
	#@echo reset >> $(SCRIPTDIR)/set_config
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/set_config

spi_$(FLASH_TYPE)_partition_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/set_partition
	@echo nand erase.chip >> $(SCRIPTDIR)/set_partition
	@echo ubi part UBI >> $(SCRIPTDIR)/set_partition
	@echo -e $(foreach n,$(TARGET_FS),ubi create $(n) $($(n)$(PATSIZE))\\n) >> $(SCRIPTDIR)/set_partition
	@echo saveenv >> $(SCRIPTDIR)/set_partition
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/set_partition

spi_$(FLASH_TYPE)_config_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/set_config
	@echo setenv bootargs \' $(rootfs$(BOOTENV)) $(kernel$(BOOTENV)) $(EXBOOTARGS) >> $(SCRIPTDIR)/set_config
	@echo setenv bootcmd \' $(wifi24mclkcmd)\; $(wifirstoffcmd)\; nand read.e $(KERNELBOOTADDR) KERNEL $(kernel$(PATSIZE))\; $(wifirstoncmd)\; bootm $(KERNELBOOTADDR)\;nand read.e $(KERNELBOOTADDR) RECOVERY $(kernel$(PATSIZE))\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/set_config
	@echo setenv autoestart 0 >> $(SCRIPTDIR)/set_config
	@echo saveenv >> $(SCRIPTDIR)/set_config
	@echo saveenv >> $(SCRIPTDIR)/set_config
	@echo reset >> $(SCRIPTDIR)/set_config
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/set_config
