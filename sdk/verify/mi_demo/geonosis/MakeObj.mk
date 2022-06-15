
export CUR_DIR= $(MODULE_OBJ_SRC)
############################################################ source files.
SUBDIRS := $(MODULE_OBJ_SRC)
OUTPUT_DIR := ./obj/$(MODULE_OBJ)

############################################################ depnedent header files.
-include $(MODULE_OBJ_SRC)/$(MODULE_OBJ).mk
include ./dirs.mk
include $(DB_BUILD_TOP)/compile.mk

all: gen_obj
clean: clean_files