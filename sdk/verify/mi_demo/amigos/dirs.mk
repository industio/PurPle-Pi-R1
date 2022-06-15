LINK_TYPE ?= static
INTER_LINK_TYPE ?= static

INC  += $(PROJ_ROOT)/release/include
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include

INC  += $(DB_BUILD_TOP)/../$(TARGET_OUT)/internal/common
LIBS += -lrt -lpthread -lm -ldl

LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/$(LINK_TYPE)

LIBS += -L./lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(INTER_LINK_TYPE)/

ifeq ($(DUAL_OS), on)
CODEDEFINE += -DLINUX_FLOW_ON_DUAL_OS
endif

-include $(MODULE)/dep.mk
LIBS := $(foreach m,$(DEP),-l$(m)) $(LIBS)
INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
INC += $(foreach m,$(3RD_PARTY_DEP),../3rdparty/$(m)/include)
LIBS += $(foreach m,$(3RD_PARTY_DEP),-l$(m))
LIBS += $(foreach m,$(3RD_PARTY_DEP),-L../3rdparty/$(m)/lib/$(LINK_TYPE))