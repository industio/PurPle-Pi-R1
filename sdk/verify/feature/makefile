-include ../../../project/configs/current.configs
-include ../../configs/current.configs
PROJ_ROOT?=$(PDIR)
include $(PROJ_ROOT)/configs/current.configs
include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk
include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
fetch-value=$(foreach n,$1,$($(n)) $($(n)enable))

INTERFACE_MODULES:=
CONFIG_LIST:=

include $(wildcard */config.mk) $(wildcard $(PROJ_ROOT)/../sdk/interface/src/*/config.mk)

INTERFACE_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(INTERFACE_MODULES),$(n)_$(interface_$(n))_))))
INTERFACE_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))
LEVEL1_CONFIG:=$(patsubst %_enable_,%,$(filter-out %__,$(filter-out %_disable_, $(foreach n,$(CONFIG_LIST),$(n)_$(verify_$(n))_))))
VERIFY_ENABLED:=$(LEVEL1_CONFIG)
VERIFY_DISABLED:=$(filter-out $(VERIFY_ENABLED),$(CONFIG_LIST))
DEP_OBJS:=$(foreach n,$(LEVEL1_CONFIG),$(patsubst %.c,$(n)/%.o,$($(n)_SRCS)))
#process cpp file
DEP_CPP_OBJS:=$(foreach n,$(LEVEL1_CONFIG),$(patsubst %.cpp,$(n)/%.o,$($(n)_SRCS_CPP)))
#end

CONFIG_LIST:=
include $(wildcard $(foreach n,$(LEVEL1_CONFIG),$(n)/*/config.mk))
LEVEL2_CONFIG:=$(CONFIG_LIST)
DEP_OBJS+=$(foreach n,$(LEVEL2_CONFIG),$(patsubst %.c,$(n)/%.o,$($(n)_SRCS)))
#process cpp file
DEP_CPP_OBJS+=$(foreach n,$(LEVEL2_CONFIG),$(patsubst %.cpp,$(n)/%.o,$($(n)_SRCS_CPP)))
#end

.PHONY: $(LEVEL1_CONFIG) $(LEVEL2_CONFIG)

CONFIG_CLEAN_LIST:=$(foreach n,$(LEVEL1_CONFIG) $(LEVEL2_CONFIG),$(n)_clean)
CLEAN_TARGET:=$(foreach n,$(LEVEL1_CONFIG) $(LEVEL2_CONFIG),$(n)/$($(n)_COMPILE_TARGET)_clean)

CONFIG_INSTALL_LIST:=$(patsubst %/bin,%_install,$(filter %/bin,$(foreach n,$(LEVEL1_CONFIG) $(LEVEL2_CONFIG),$(n)/$($(n)_COMPILE_TARGET))))
#process cpp file
CONFIG_INSTALL_LIST+=$(patsubst %/cpp_bin,%_install,$(filter %/cpp_bin,$(foreach n,$(LEVEL1_CONFIG) $(LEVEL2_CONFIG),$(n)/$($(n)_COMPILE_TARGET))))
#end

LEVEL1_TARGET:=$(foreach n,$(LEVEL1_CONFIG),$(n)/$($(n)_COMPILE_TARGET))
LEVEL2_TARGET:=$(foreach n,$(LEVEL2_CONFIG),$(n)/$($(n)_COMPILE_TARGET))

all: $(LEVEL1_CONFIG)

install: $(CONFIG_INSTALL_LIST)

clean: $(CONFIG_CLEAN_LIST)

$(DEP_OBJS):%.o:%.c
	$(CC) -c $^ -o $@ -I$(dir $@) -I$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/include/uapi/mstar -I$(PROJ_ROOT)/release/include $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_DEPS) $($(patsubst %/$(notdir $@),%,$@)_DEP_INCS),-I$(n)) -g $($(patsubst %/$(notdir $@),%,$@)_CFLAGS) $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(INTERFACE_DISABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=0) $(foreach n,$(VERIFY_ENABLED),-DVERIFY_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(VERIFY_DISABLED),-DVERIFY_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)

#process cpp file
$(DEP_CPP_OBJS):%.o:%.cpp
	$(CXX) -c $^ -o $@ -I$(dir $@) -I$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar -I$(PROJ_ROOT)/release/include $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_DEPS) $($(patsubst %/$(notdir $@),%,$@)_DEP_INCS),-I$(n)) -g $($(patsubst %/$(notdir $@),%,$@)_CFLAGS) $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(INTERFACE_DISABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))=0) $(foreach n,$(VERIFY_ENABLED),-DVERIFY_$(shell tr 'a-z' 'A-Z' <<< $(n))=1) $(foreach n,$(VERIFY_DISABLED),-DVERIFY_$(shell tr 'a-z' 'A-Z' <<< $(n))=0)
#end

%/bin:
	$(MAKE) $(patsubst %,%/lib,$($(patsubst %/$(notdir $@),%,$@)_DEPS)) do_noting
	$(MAKE) $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS))
	$(CC) $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS)) -o $(patsubst %/bin,%/prog,$@) $< -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/static -L$(dir $@) -Wl,--start-group -lpthread -lrt -lmi_sys $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_LIBS),-l$(n)) $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_DEPS),$(n)/lib.a) -Wl,--end-group

#process cpp execute
%/cpp_bin:
	$(MAKE) $(patsubst %,%/cpp_lib,$($(patsubst %/$(notdir $@),%,$@)_DEPS_CPP)) do_noting
	$(MAKE) $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP))
	$(CXX) $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP)) -o $(patsubst %/cpp_bin,%/prog,$@) $< -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/static -L$(dir $@) -Wl,--start-group -lpthread -lrt -lmi_sys $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_LIBS),-l$(n)) $(foreach n,$($(patsubst %/$(notdir $@),%,$@)_DEPS_CPP),$(n)/lib.a) -Wl,--end-group
#end

%/bin_clean:
	$(MAKE) $(patsubst %,%/lib_clean,$($(patsubst %/$(notdir $@),%,$@)_DEPS)) do_noting
	rm -f $(patsubst %/bin_clean,%/prog,$@) $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS))

#process cpp clean
%/cpp_bin_clean:
	$(MAKE) $(patsubst %,%/cpp_lib_clean,$($(patsubst %/$(notdir $@),%,$@)_DEPS_CPP)) do_noting
	rm -f $(patsubst %/cpp_bin_clean,%/prog,$@) $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP))
#end
%/lib:
	$(MAKE) $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS))
	$(AR) rc $@.a $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS))

#process cpp lib
%/cpp_lib:
	$(MAKE) $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP))
	$(AR) rc $@.a $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP))
#end

%/lib_clean:
	rm -f $(patsubst %/lib_clean,%/lib.a,$@) $(patsubst %.c,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS))

#process cpp lib clean
%/cpp_lib_clean:
	rm -f $(patsubst %/cpp_lib_clean,%/lib.a,$@) $(patsubst %.cpp,$(dir $@)%.o,$($(patsubst %/$(notdir $@),%,$@)_SRCS_CPP))
#end

%/nop:
	@#skip

%/nop_clean:
	@#skip

$(LEVEL1_CONFIG): %:
	$(MAKE) $(filter $@/%,$(LEVEL2_TARGET)) do_noting
	$(MAKE) $(filter $@/%,$(LEVEL1_TARGET)) do_noting

$(LEVEL2_CONFIG): %:
	$(MAKE) $(filter $@/%,$(LEVEL2_TARGET)) do_noting

$(CONFIG_CLEAN_LIST): %_clean:
	$(MAKE) $(filter $(patsubst %_clean,%,$@)/%, $(CLEAN_TARGET))

$(CONFIG_INSTALL_LIST): %_install:
	mkdir -p $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/verify/$(dir $@)
	cp $(patsubst %_install,%,$@)/prog $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/verify/$(dir $@)feature_$(notdir $(patsubst %_install,%,$@))

do_noting:
	@#do nothing
