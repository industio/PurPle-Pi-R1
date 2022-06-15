-include ./$(MODULE)/$(MODULE).mk
include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

install:
	@$(STRIP) --strip-unneeded ../out/$(DB_PROJECT)/app/prog_$(MODULE)
	@mkdir -p $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/$(MODULE)/
	@cp -vrf ../out/$(DB_PROJECT)/app/prog_$(MODULE) $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/$(MODULE)/
ifneq ($(MODULE_REL_FILES), )
	$(foreach n,$(MODULE_REL_FILES),cp -rfv $(n) $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/$(MODULE)/;)
endif