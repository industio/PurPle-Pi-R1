OUTPUT_DIR := ./lib

OBJ_FILES := ./obj/$(MODULE_LIB)/*.o
LIB_NAME=st_$(MODULE_LIB)
LIB_TYPE=static

include $(DB_BUILD_TOP)/compile.mk

all: gen_lib
clean: clean_files