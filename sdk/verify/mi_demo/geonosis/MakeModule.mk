-include $(MODULE)/$(MODULE).mk

DEP_LIB_ALL := $(foreach m,$(ST_DEP),$(m)_lib_all)
DEP_LIB_CLEAN := $(foreach m,$(ST_DEP),$(m)_lib_clean)
DEP_OBJ_ALL := $(foreach m,$(ST_DEP),$(m)_obj_all)
DEP_OBJ_CLEAN := $(foreach m,$(ST_DEP),$(m)_obj_clean)

.PHONY : all clean $(DEP_LIB_ALL) $(DEP_LIB_CLEAN) $(DEP_OBJ_ALL) $(DEP_OBJ_CLEAN)

all : $(DEP_LIB_ALL)
clean : $(DEP_OBJ_CLEAN) $(DEP_LIB_CLEAN)

$(DEP_LIB_ALL): $(DEP_OBJ_ALL)

$(DEP_OBJ_ALL):
	$(MAKE) $@
$(DEP_LIB_ALL):
	$(MAKE) $@
$(DEP_OBJ_CLEAN):
	$(MAKE) $@
$(DEP_LIB_CLEAN):
	$(MAKE) $@