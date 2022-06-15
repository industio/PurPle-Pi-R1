OBJ_FILES := ./obj/$(MODULE_LIB)/*.o
LIB_NAME:=$(MODULE_LIB)
LIB_TYPE?=static
OUTPUT_DIR := ./lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(LIB_TYPE)

include $(DB_BUILD_TOP)/compile.mk

all: gen_lib
clean: clean_files