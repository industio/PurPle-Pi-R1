DB_PROJECT:=demo
DB_BUILD_TOP:=$(shell pwd)
DB_OUT_PATH:=$(DB_BUILD_TOP)/../out/$(DB_PROJECT)
DB_LIB_TYPE:=static
DB_ALKAID_PROJ:=$(DB_BUILD_TOP)/../../../../project/configs/current.configs
include $(DB_ALKAID_PROJ)
include $(PROJ_ROOT)/configs/current.configs

ifeq ($(PRODUCT), nvr)
	MODULES := fb vdec audio_all_test_case hdmi_test cardv_test
endif
LIBS_PATH := ./internal/ ../common/

INTERNAL_LIBS := $(notdir $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)
MODULES_INSTALL := $(foreach m,$(MODULES),$(m)_install)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED
export DB_BUILD_TOP
export DB_OUT_PATH
export DB_PROJECT
export DB_ALKAID_PROJ
export LIBS_PATH
.PHONY: all lib obj clean $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

all:
	$(foreach mod,$(MODULES),$(MAKE) $(mod);)
clean:
	$(foreach mod,$(MODULES_OBJS_CLEAN),$(MAKE) $(mod);)
	$(foreach mod,$(MODULES_LIBS_CLEAN),$(MAKE) $(mod);)
	$(foreach mod,$(MODULES_APP_CLEAN),$(MAKE) $(mod);)
	@rm -rvf MakefileTmp_*

install:
	@$(STRIP) --strip-unneeded ../out/$(DB_PROJECT)/app/*
	@mkdir -p $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
	$(foreach mod,$(MODULES_INSTALL),$(MAKE) $(mod);)

$(MODULES_INSTALL):
	@$(MAKE) MODULE=$(patsubst %_install,%,$@) -f ./MakeInstall.mk install

$(MODULES):
	@$(MAKE) MODULE=$@ -f ./MakeModule.mk all
	@$(MAKE) $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULE=$(patsubst %_clean,%,$@) -f ./MakeModule.mk clean

$(MODULES_APP_ALL):
	@$(MAKE) MODULE=$(patsubst %_app_all,%,$@) -f ./MakeApp.mk all

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE=$(patsubst %_app_clean,%,$@) -f ./MakeApp.mk clean

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_all,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk all

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_clean,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_clean,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk clean

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_all,%,$@) -f ./MakeLib.mk all

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_clean,%,$@) -f ./MakeLib.mk clean
