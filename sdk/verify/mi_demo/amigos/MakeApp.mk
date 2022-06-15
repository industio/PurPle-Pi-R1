############################################################ source files.
SUBDIRS:=$(DB_BUILD_TOP)/$(MODULE)

EXEFILE    := $(MODULE)
OUTPUT_DIR := $(DB_OUT_PATH)/app

############################################################ depnedent header files.
export MOD_ROOT=./$(MODULE)
-include $(DB_BUILD_TOP)/$(MODULE)/$(MODULE).mk
include $(DB_BUILD_TOP)/dirs.mk
include $(DB_BUILD_TOP)/compile.mk

all: gen_exe
clean: clean_files
