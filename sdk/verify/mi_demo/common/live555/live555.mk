INC += $(CUR_DIR)/BasicUsageEnvironment/include
INC += $(CUR_DIR)/groupsock/include
INC += $(CUR_DIR)/liveMedia/include
INC += $(CUR_DIR)/UsageEnvironment/include
INC += $(CUR_DIR)/mediaServer/include

SUBDIRS += $(CUR_DIR)/BasicUsageEnvironment
SUBDIRS += $(CUR_DIR)/groupsock
SUBDIRS += $(CUR_DIR)/liveMedia
SUBDIRS += $(CUR_DIR)/UsageEnvironment
SUBDIRS += $(CUR_DIR)/mediaServer

LOCAL_CXXFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DBSD=1 -DLOCALE_NOT_USED -DSO_REUSEPORT -DALLOW_SERVER_PORT_REUSE -DALLOW_RTSP_SERVER_PORT_REUSE