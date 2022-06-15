include $(DB_BUILD_TOP)/mi_dep.mk

LINK_TYPE := static
INTER_LINK_TYPE := static

INC  += $(DB_BUILD_TOP)/../common/live555/UsageEnvironment/include
INC  += $(DB_BUILD_TOP)/../common/live555/groupsock/include
INC  += $(DB_BUILD_TOP)/../common/live555/liveMedia/include
INC  += $(DB_BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC  += $(DB_BUILD_TOP)/../common/live555/mediaServer/include
INC  += $(DB_BUILD_TOP)/internal/base/include/


ifneq ($(interface_vif), disable)
LIBS += -lmi_vif
endif

ifneq ($(interface_vpe), disable)
LIBS += -lmi_vpe
endif

ifneq ($(interface_venc), disable)
LIBS += -lmi_venc
endif

ifneq ($(interface_rgn), disable)
LIBS += -lmi_rgn
endif

ifneq ($(interface_divp), disable)
LIBS += -lmi_divp
endif

ifneq ($(interface_isp), disable)
LIBS += -lmi_isp -lcus3a -lispalgo
endif

ifneq ($(interface_disp), disable)
#LIBS += -lmi_disp
endif

ifneq ($(interface_panel), disable)
LIBS += -lmi_panel
endif

ifneq ($(interface_iqserver), disable)
LIBS += -lmi_iqserver
endif

ifneq ($(interface_sensor), disable)
LIBS += -lmi_sensor
endif

ifneq ($(interface_vdec), disable)
LIBS += -lmi_vdec
endif

ifneq ($(interface_vdisp), disable)
LIBS += -lmi_vdisp
endif

ifneq ($(interface_ai), disable)
LIBS += -lmi_ai
endif

ifneq ($(interface_ao), disable)
LIBS += -lmi_ao
endif

LIBS += -lmi_sys -lcam_fs_wrapper -lcam_os_wrapper



