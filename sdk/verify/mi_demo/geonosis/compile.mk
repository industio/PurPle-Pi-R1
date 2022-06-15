.PHONY :all clean gen_exe gen_obj clean_files gen_lib

include $(DB_ALKAID_PROJ)

include $(PROJ_ROOT)/configs/current.configs
include $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/toolchain.mk

#GCCFLAGS := -Wall -g -Werror
GCCFLAGS := -Wall -g -mthumb -pipe -fPIC
CXXFLAGS := $(GCCFLAGS) $(LOCAL_CXXFLAGS)
CXXFLAGS += $(CODEDEFINE) -DLINUX_OS -std=c++11
CXXFLAGS += $(foreach dir,$(INC),-I$(dir))

CFLAGS := $(GCCFLAGS) $(LOCAL_CFLAGS)
CFLAGS += $(CODEDEFINE) -DLINUX_OS
CFLAGS += $(foreach dir,$(INC),-I$(dir))

SRC_CXX  :=  $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp))
SRC      :=  $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
OBJS_CXX := $(patsubst %.cpp,%.o,$(SRC_CXX))
OBJS     := $(patsubst %.c,%.o,$(SRC))

DFILES := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.d))
sinclude $(DFILES)
#@$(CC) $(CFLAGS) -MM $< | sed -e 's/\(.*\)\.o/\$$\(SUBDIRS\)\/\1.o/g' > $(@:.o=.d)

$(OBJS):%.o:%.c
	@echo compile $<...
	@$(CC) $(CFLAGS) -MM $< | sed -e '1s/'"^.*"'/'"$(subst /,"'\/'",$@) : $(subst /,"'\/'",$(@:.o=.d)) $(subst /,"'\/'",$<)"'\\''/' > $(@:.o=.d)
	@$(CC) $(CFLAGS) -c -ffunction-sections -fdata-sections $< -o $@
$(OBJS_CXX):%.o:%.cpp
	@echo compile $<...
	@$(CXX) $(CXXFLAGS) -MM $< | sed -e '1s/'"^.*"'/'"$(subst /,"'\/'",$@) : $(subst /,"'\/'",$(@:.o=.d)) $(subst /,"'\/'",$<)"'\\''/' > $(@:.o=.d)
	@$(CXX) $(CXXFLAGS) -c -ffunction-sections -fdata-sections $< -o $@

gen_exe:$(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@$(CXX) $(CXXFLAGS) -Wl,--gc-sections $(OBJS_CXX) $(OBJS) $(LIBS) -o $(EXEFILE)
	@mkdir -p $(OUTPUT_DIR)
	@mv $(EXEFILE) $(OUTPUT_DIR) -v
endif
ifneq ($(OBJS), )
	@$(CC) $(CFLAGS) -Wl,--gc-sections $(OBJS) $(LIBS) -o $(EXEFILE)
	@mkdir -p $(OUTPUT_DIR)
	@mv $(EXEFILE) $(OUTPUT_DIR) -v
endif

gen_obj:$(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@mkdir -p $(OUTPUT_DIR)
	@cp $(OBJS_CXX) $(OUTPUT_DIR) -v
endif
ifneq ($(OBJS), )
	@mkdir -p $(OUTPUT_DIR)
	@cp $(OBJS) $(OUTPUT_DIR) -v
endif

gen_lib:
ifeq ($(LIB_TYPE), static)
	@$(AR) sq lib$(LIB_NAME).a $(OBJ_FILES)
ifneq ($(OUTPUT_DIR), )
	@mkdir -p $(OUTPUT_DIR)
	@mv ./lib$(LIB_NAME).a $(OUTPUT_DIR) -v
endif
endif
ifeq ($(LIB_TYPE), shared)
	$(CC) -shared -fPIC -o lib$(LIB_NAME).so $(OBJ_FILES)
ifneq ($(OUTPUT_DIR), )
	@mkdir -p $(OUTPUT_DIR)
	@mv ./lib$(LIB_NAME).so $(OUTPUT_DIR) -v
endif
endif
ifeq ($(LIB_TYPE), shared_cxx)
	$(CXX) -shared -fPIC -o lib$(LIB_NAME).so $(OBJ_FILES)
ifneq ($(OUTPUT_DIR), )
	@mkdir -p $(OUTPUT_DIR)
	@mv ./lib$(LIB_NAME).so $(OUTPUT_DIR) -v
endif
endif
ifneq ($(LIB_TYPE), )
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	@echo lib$(LIB_NAME) is ready!!!!!!!
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
else
	@echo "Error LIB_TYPE not set!!"
endif

clean_files:
	@rm -rvf $(OBJS_CXX) $(OBJS) $(OBJS_CXX:.o=.d) $(OBJS:.o=.d)
	@rm -rvf $(OUTPUT_DIR)
ifneq ($(EXEFILE), )
	@rm -rvf $(OUTPUT_DIR)/$(EXEFILE)
endif
