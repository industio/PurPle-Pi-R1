include $(DB_BUILD_TOP)/mi_dep.mk

INC  += $(DB_BUILD_TOP)/../common/live555/UsageEnvironment/include
INC  += $(DB_BUILD_TOP)/../common/live555/groupsock/include
INC  += $(DB_BUILD_TOP)/../common/live555/liveMedia/include
INC  += $(DB_BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC  += $(DB_BUILD_TOP)/../common/live555/mediaServer/include
INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
INC += $(CUR_DIR)/include/

ifneq ($(interface_rgn), disable)
SUBDIRS += $(CUR_DIR)/src/ui
endif
ifneq ($(interface_vpe), disable)
SUBDIRS += $(CUR_DIR)/src/vpe
endif
ifneq ($(interface_vif), disable)
SUBDIRS += $(CUR_DIR)/src/vif
endif
ifneq ($(interface_divp), disable)
SUBDIRS += $(CUR_DIR)/src/divp
endif
ifneq ($(interface_venc), disable)
SUBDIRS += $(CUR_DIR)/src/venc
endif
ifneq ($(interface_disp), disable)
SUBDIRS += $(CUR_DIR)/src/disp
endif
ifneq ($(interface_iqserver), disable)
SUBDIRS += $(CUR_DIR)/src/iq
endif
ifneq ($(interface_vdec), disable)
SUBDIRS += $(CUR_DIR)/src/vdec
endif
ifneq ($(interface_vdisp), disable)
SUBDIRS += $(CUR_DIR)/src/vdisp
endif
ifneq ($(interface_ai), disable)
SUBDIRS += $(CUR_DIR)/src/ai
endif
ifneq ($(interface_ao), disable)
SUBDIRS += $(CUR_DIR)/src/ao
endif

SUBDIRS += $(CUR_DIR)/src/file
SUBDIRS += $(CUR_DIR)/src/rtsp
SUBDIRS += $(CUR_DIR)/src/sys
SUBDIRS += $(CUR_DIR)/src/dla
