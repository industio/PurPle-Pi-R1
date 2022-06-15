TARGET_OTA_IMAGE:=$(foreach n,$(OTA_IMAGE_LIST),$(n)_$(FLASH_TYPE)_mkota_$($(n)$(FSTYPE)))

define makeota
@read -p "Make $(1) ?(yes/no)" select;	\
if [ "$${select}" == "yes" -o "$${select}" == "y" ]; then	\
	$(foreach n,$(3),$(PROJ_ROOT)/image/makefiletools/bin/otapack -s $(2) -d $(n) -t $(4) -m $(5) $(6) -a $(IMAGEDIR)/SStarOta.bin;) \
fi;
endef

ota_images: ota_image_creat $(TARGET_OTA_IMAGE) ota_image_compressed

ota_image_creat:
ifneq ($(TARGET_OTA_IMAGE), )
	@read -p "Start scripts:" start_scripts;	\
	read -p "End scripts:" end_scripts;	\
	if [ "$${start_scripts}" != "" -a "$${end_scripts}" != "" ]; then	\
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -b $${start_scripts} -e $${end_scripts}; \
	elif [ "$${start_scripts}" != "" -a "$${end_scripts}" == "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -b $${start_scripts}; \
	elif [ "$${start_scripts}" == "" -a "$${end_scripts}" != "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin -e $${end_scripts}; \
	elif [ "$${start_scripts}" == "" -a "$${end_scripts}" == "" ]; then   \
		$(PROJ_ROOT)/image/makefiletools/bin/otapack -c $(IMAGEDIR)/SStarOta.bin; \
	fi;
endif

ota_image_compressed:
ifneq ($(TARGET_OTA_IMAGE), )
	@gzip $(IMAGEDIR)/SStarOta.bin
endif

ipl_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/ipl_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update)

ipl_cust_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/ipl_cust_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update)

uboot_spinand_mkota_:
	$(call makeota,$(patsubst %_spinand_mkota_,%,$@),$(IMAGEDIR)/uboot_s.bin,$($(patsubst %_spinand_mkota_,%,$@)$(OTABLK)),$($(patsubst %_spinand_mkota_,%,$@)$(PATSIZE)),0,--block-update)

boot_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/boot.bin,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update)

logo_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/logo,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update)

kernel_$(FLASH_TYPE)_mkota_:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_,%,$@),$(IMAGEDIR)/kernel,$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_,%,$@)$(PATSIZE)),0,--block-update)

rootfs_$(FLASH_TYPE)_mkota_ramfs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_ramfs,%,$@),$(IMAGEDIR)/rootfs.ramfs,$($(patsubst %_$(FLASH_TYPE)_mkota_ramfs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_ramfs,%,$@)$(PATSIZE)),0,--block-update)

%_$(FLASH_TYPE)_mkota_squashfs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@).sqfs,$($(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_squashfs,%,$@)$(PATSIZE)),0,--block-update)
%_$(FLASH_TYPE)_mkota_jffs2:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@).jffs2,$($(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_jffs2,%,$@)$(PATSIZE)),0,--block-update)
%_$(FLASH_TYPE)_mkota_ubifs:
	$(call makeota,$(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@),$(IMAGEDIR)/$(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@).ubifs,$($(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@)$(OTABLK)),$($(patsubst %_$(FLASH_TYPE)_mkota_ubifs,%,$@)$(PATSIZE)),0,--ubi-update)
