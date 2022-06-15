BASEDIR?=$(CURDIR)
include $(BASEDIR)/../../../misc/misc.mk

BIN=fblogo

INCS =  $(PROJ_ROOT)/release/include \
		3rdParty/include \
		$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/include/uapi/mstar 

SUBDIRS = sstardisp raw jpeg bmp png  

INCS += $(SUBDIRS)
SRCS:=
SRCS += $(foreach n,$(SUBDIRS),$(wildcard $(n)/*.c))
SRCS += ${wildcard *.c}
CFLAGS = -rdynamic -funwind-tables -D_GNU_SOURCE
CLFAGS += -Wall -Werror -g
CFLAGS += $(patsubst %,-I%,$(INCS))

LDFLAGS = -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic
LDFLAGS += -L3rdParty/lib
LDFLAGS += -lmi_common -lmi_sys -lmi_disp -lmi_panel
ifneq ($(wildcard $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic/libmi_hdmi.so),)
LDFLAGS += -lmi_hdmi
endif
LDFLAGS += -lpthread -ldl -ljpeg -lpng -lz
OBJS=$(patsubst %.c,%.o,$(SRCS))

.PHONY:
all:$(OBJS)
	$(CC) $(LDFLAGS) -o  $(BIN) $^
	@$(patsubst %gcc,%strip,$(CC)) --strip-unneeded $(BIN)

clean:
	rm -rf $(OBJS) $(BIN)

install:
	@echo done!

%.o:%.c 
	@echo  CC $@
	@$(CC) -c -o $@ $^  $(CFLAGS)