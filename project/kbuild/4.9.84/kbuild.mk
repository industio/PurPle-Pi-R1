.PHONY: symbol_link symbol_link_clean
include configs/current.configs
include image/configs/$(CHIP)/$(IMAGE_CONFIG)

symbol_link:
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/arch $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/drivers $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/include $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/scripts $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/Makefile $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/include/* $(CURDIR)/kbuild/$(KERNEL_VERSION)/include/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/Module.symvers $(CURDIR)/kbuild/$(KERNEL_VERSION)/
	ln -sf $(CURDIR)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/.config $(CURDIR)/kbuild/$(KERNEL_VERSION)/

symbol_link_clean:
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/include/config -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/include/generated -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/Module.symvers -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/.config -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/Makefile -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/scripts -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/include -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/drivers -rfv
	rm $(CURDIR)/kbuild/$(KERNEL_VERSION)/arch -rfv
