LINK_TYPE ?= dynamic

INC  += $(PROJ_ROOT)/release/include
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include

INC  += $(DB_BUILD_TOP)/internal/common
LIBS += -lrt -lpthread -lm -ldl
#
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/$(LINK_TYPE)
LIBS += -L./lib
