DB_BUILD_TOP:=$(shell pwd)
DB_OUT_PATH:=$(DB_BUILD_TOP)/out
DB_ALKAID_PROJ:=$(DB_BUILD_TOP)/../../../../project/configs/current.configs
DB_3PARTY_PATH:=$(DB_BUILD_TOP)/../3rdparty
include $(DB_ALKAID_PROJ)
-include $(MODULE)/$(MODULE).mk
ifeq ($(CHIP), i6)
TARGET_OUT := hoth
endif
ifeq ($(CHIP), i6e)
TARGET_OUT := alderaan
endif
ifeq ($(CHIP), i6b0)
TARGET_OUT := jedi
endif

ifeq ($(CHIP), i2m)
TARGET_OUT := geonosis
endif

MODULES := preview
ifeq ($(PRODUCT), ipc)
MODULES += hdmi_convertor 
endif

LIBS_PATH := ../$(TARGET_OUT)/internal/ ../common/ ./internal/

INTERNAL_LIBS := $(notdir $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)
3RD_PARTY := $(foreach m,$(notdir $(wildcard $(DB_3PARTY_PATH)/*)),$(m)_3rd_party_all)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS
export DB_BUILD_TOP
export DB_OUT_PATH
export DB_PROJECT
export DB_ALKAID_PROJ
export TARGET_OUT
export LIBS_PATH
export DB_3PARTY_PATH

.PHONY: all lib obj clean install depend_internal depend_internal_clean depend_3rd_party $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

-include $(foreach mod,$(MODULES_IN),$(mod)/dep.mk)
all:
	$(MAKE) MODULES_IN="$(MODULES)" depend_3rd_party
	$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	$(MAKE) $(MODULES_APP_ALL)
endif

clean:
	$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_clean,$(mod)))
	$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
	@rm -rvf MakefileTmp_*
	@rm -rvf $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/prog_*
	@rm $(DB_OUT_PATH) -rf
	@rm ./obj/ -rf

install:
	@$(STRIP) --strip-unneeded ../out/$(DB_PROJECT)/app/*
	@mkdir -p $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
	@cp -vrf ../out/$(DB_PROJECT)/app/* $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
	@mkdir -p ../$(TARGET_OUT)/internal/amigos/lib/
	@mkdir -p ../$(TARGET_OUT)/internal/amigos/include/
	@cp -vrf ./lib/libbase.a ../$(TARGET_OUT)/internal/amigos/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/libamigos.a
	@cp -vrf ./internal/base/include/* ../$(TARGET_OUT)/internal/amigos/include/
depend_internal:
ifneq ($(DEP), )
	$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_all)
	$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_all)
endif

depend_internal_clean:
ifneq ($(DEP), )
	$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_clean)
	$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_clean)
endif

depend_3rd_party:
ifneq ($(3RD_PARTY_DEP), )
	$(MAKE) $(foreach m,$(sort $(3RD_PARTY_DEP)),$(m)_3rd_party_all)
endif
$(3RD_PARTY):
	$(MAKE) -C ../3rdparty/$(patsubst %_3rd_party_all,%,$@)

$(MODULES):
	$(MAKE) MODULES_IN=$@ depend_3rd_party
	$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$@ depend_internal_clean

$(MODULES_APP_ALL):
	@$(MAKE) MODULE=$(patsubst %_app_all,%,$@) -f ./MakeApp.mk all

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE=$(patsubst %_app_clean,%,$@) -f ./MakeApp.mk clean

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_all,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk all

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_clean,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_clean,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk clean

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_all,%,$@) LIB_TYPE=static -f ./MakeLib.mk all
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_all,%,$@) LIB_TYPE=dynamic -f ./MakeLib.mk all

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_clean,%,$@) LIB_TYPE=static -f ./MakeLib.mk clean
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_clean,%,$@) LIB_TYPE=dynamic -f ./MakeLib.mk clean
