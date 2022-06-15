CONFIG_MK := $(firstword $(dir $(word $(shell echo $$(($(words $(MAKEFILE_LIST))-2))), $(MAKEFILE_LIST))))
MODULE_NAME:=$(patsubst %/,%, $(filter-out $(PROJ_ROOT)/../sdk/interface%,$(CONFIG_MK)))
INTERFACE_MODULES += $(notdir $(patsubst $(PROJ_ROOT)/../sdk/interface/src/%/,%,$(filter $(PROJ_ROOT)/../sdk/interface%, $(CONFIG_MK))))
CONFIG_LIST += $(MODULE_NAME)
$(MODULE_NAME)_DEPS := $(call fetch-value,DEPS)
$(MODULE_NAME)_DEP_INCS := $(call fetch-value,DEP_INCS)
$(MODULE_NAME)_COMPILE_TARGET := $(COMPILE_TARGET)
$(MODULE_NAME)_CFLAGS := $(call fetch-value,CFLAGS)
$(MODULE_NAME)_LIBS := $(call fetch-value,LIBS)
$(MODULE_NAME)_SRCS := $(call fetch-value,SRCS)
#process cpp file
$(MODULE_NAME)_SRCS_CPP := $(call fetch-value,SRCS_CPP)
$(MODULE_NAME)_DEPS_CPP := $(call fetch-value,DEPS_CPP)
#end
