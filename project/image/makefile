OUTPUTDIR:=$(CURDIR)/output
IMAGEDIR:=$(CURDIR)/output/images
RESOUCE:=_resource
FSTYPE:=_fstype
PATSIZE:=_patsize
MOUNTPT:=_mountpoint
MOUNTTG:=_mounttarget
OPTIONS:=_options
BOOTENV:=_bootenv
MTDPART:=_mtdpart
BOOTTAB:=_boottab
SYSTAB:=_systab
CISSIZE:=_cissize
BOOTADDR:=_bootaddr
COPIES:=_copies
DATASIZE:=_datasize
BKCOUNT:=_bkcount
BLKSIZE:=_blksize
PATCOUNT:=_patcount
PGSIZE:=_pgsize
OTABLK:=_otablk

include ../configs/current.configs
include ../release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk
include configs/$(CHIP)/$(IMAGE_CONFIG)

define multiplyhex
$(shell printf "0x%x" $(shell echo $$[$(1)*$(2)]))
endef

ifeq ($(FLASH_TYPE), nor)
	include configs/$(CHIP)/script_nor.mk
else ifeq ($(FLASH_TYPE), emmc)
	include configs/$(CHIP)/script_emmc.mk
else
	# for spinand or spinand-ramdisk
	include configs/$(CHIP)/script_nand.mk
endif

ifeq ($(FLASH_TYPE), spinand-ramdisk)
	include configs/$(CHIP)/ramdisk.mk
endif

ifeq ($(BOOT_TYPE), fastboot)
	include configs/$(CHIP)/rootfs_fastboot.mk
else
	include configs/$(CHIP)/rootfs.mk
endif
-include configs/$(CHIP)/rootfs_recovery.mk
include image.mk
include boot.mk
include ota.mk

PERF:=$(patsubst %,tar xf build/%.tar.gz -C $(OUTPUTDIR)/rootfs,$(PERF_NAME))
STAP:=$(patsubst %,tar xf build/%.tar.gz -C $(OUTPUTDIR)/rootfs,$(STAP_NAME))

all:
	@mkdir -p $(OUTPUTDIR)
	@mkdir -p $(IMAGEDIR)
	@mkdir -p $(IMAGEDIR)/boot
ifeq ($(FLASH_TYPE), spinand-ramdisk)
	$(MAKE) ramdisk
endif
	$(MAKE) rootfs
	$(MAKE) rootfs_recovery
	$(MAKE) images
	$(MAKE) boot_images
	$(MAKE) scripts

clean:
	rm -rf $(OUTPUTDIR)

