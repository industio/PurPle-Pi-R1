INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
LIBS += $(foreach m,$(ST_DEP),-lst_$(m))