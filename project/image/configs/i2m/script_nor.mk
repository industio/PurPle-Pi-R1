TARGET_SCRIPT:=$(foreach n,$(IMAGE_LIST),$(n)_$(PAT_TABLE)_$(FLASH_TYPE)_$($(n)$(FSTYPE))_script) $(PAT_TABLE)_$(FLASH_TYPE)_config_script

TARGET_FS:=$(filter-out $(patsubst %_fs__,%,$(filter %_fs__, $(foreach n,$(IMAGE_LIST),$(n)_fs_$($(n)$(FSTYPE))_))), $(IMAGE_LIST))

SCRIPTDIR:=$(IMAGEDIR)/scripts

scripts:
	mkdir -p $(SCRIPTDIR)
	$(MAKE) $(TARGET_SCRIPT)
	@echo -e $(foreach n,$(IMAGE_LIST),estar scripts/[[$(n)\\n) >> $(IMAGEDIR)/auto_update.txt
	@echo estar scripts/set_config >> $(IMAGEDIR)/auto_update.txt
	@echo saveenv >> $(IMAGEDIR)/auto_update.txt
	@echo printenv >> $(IMAGEDIR)/auto_update.txt
	@echo reset >> $(IMAGEDIR)/auto_update.txt

#for spi nor flash kernel.
kernel_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp r.info KERNEL >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_spi_nor__script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo setenv sf_kernel_start \$${sf_part_start} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo setenv sf_kernel_size \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo setenv bootcmd \' bootlogo 0 0 0 0 0\; $(wifi24mclkcmd)\; $(wifirstoffcmd)\; sf probe 0\;$(kernel$(BOOTCMD)) $(rootfs$(BOOTCMD)) $(wifirstoncmd)\; bootm $(KERNELBOOTADDR) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo setenv dispout $(DISP_OUT_NAME) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)

uboot_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[uboot
	@echo mxp r.info UBOOT >> $(SCRIPTDIR)/[[uboot
	@echo sf probe 0 >> $(SCRIPTDIR)/[[uboot
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[uboot
	@echo tftp $(TFTPDOWNLOADADDR) boot/u-boot.xz.img.bin >> $(SCRIPTDIR)/[[uboot
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[uboot
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[uboot

logo_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[logo
	@echo mxp r.info LOGO >> $(SCRIPTDIR)/[[logo
	@echo sf probe 0 >> $(SCRIPTDIR)/[[logo
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[logo
	@echo tftp $(TFTPDOWNLOADADDR) logo >> $(SCRIPTDIR)/[[logo
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[logo
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[logo

ipl_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp r.info IPL >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) boot/$(notdir $(ipl$(RESOUCE))) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)

boot_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) boot.bin >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf erase 0 \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) 0 \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp t.load >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)

ipl_cust_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp r.info IPL_CUST >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) boot/$(notdir $(ipl_cust$(RESOUCE))) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)

mxp_spi_nor__script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp t.init >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) boot/MXP_SF.bin >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp t.update $(TFTPDOWNLOADADDR) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo mxp t.load >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor__script,%,$@)


%_spi_nor_squashfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo mxp r.info $(patsubst %_spi_nor_squashfs_script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_spi_nor_squashfs_script,%,$@).sqfs >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_squashfs_script,%,$@)

%_spi_nor_ramfs_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo mxp r.info $(patsubst %_spi_nor_ramfs_script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_spi_nor_ramfs_script,%,$@).ramfs >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo setenv initrd_high ${INITRAMFSLOADADDR} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo setenv initrd_size \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_ramfs_script,%,$@)

%_spi_nor_jffs2_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo mxp r.info $(patsubst %_spi_nor_jffs2_script,%,$@) >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo sf probe 0 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo sf erase \$${sf_part_start} \$${sf_part_size} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo tftp $(TFTPDOWNLOADADDR) $(patsubst %_spi_nor_jffs2_script,%,$@).jffs2 >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo sf write $(TFTPDOWNLOADADDR) \$${sf_part_start} \$${filesize} >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/[[$(patsubst %_spi_nor_jffs2_script,%,$@)

spi_nor_config_script:
	@echo "# <- this is for comment / total file size must be less than 4KB" > $(SCRIPTDIR)/set_config
	@echo setenv bootargs console=ttyS0,115200 $(rootfs$(BOOTENV)) $(kernel$(BOOTENV)) $(EXBOOTARGS) >> $(SCRIPTDIR)/set_config
	@echo setenv autoestart 0 >> $(SCRIPTDIR)/set_config
	@echo saveenv >> $(SCRIPTDIR)/set_config
	@echo saveenv >> $(SCRIPTDIR)/set_config
	@echo reset >> $(SCRIPTDIR)/set_config
	@echo  "% <- this is end of file symbol" >> $(SCRIPTDIR)/set_config
