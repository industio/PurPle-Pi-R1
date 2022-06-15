PWD=`pwd`
SDK_LIBS=$(PROJ_ROOT)/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic
SDK_LIBS_STATIC=$(patsubst %dynamic,%static,$(SDK_LIBS))
PLAT_DEPENDENT_LIB=-lmi_gfx -lmi_sys -lmi_common -ldl 
PLAT_DEPENDENT_LIB_EX= -lmi_disp -lmi_panel 
MINIGUI_LIBS= -ljpeg -lz 
SDK_INCS=$(PROJ_ROOT)/release/include
UAPI_INCS=$(PROJ_ROOT)/kbuild/4.9.84/i2m/include/uapi/mstar
LDFLAGS+=-L$(SDK_LIBS)  -L$(SDK_LIBS_STATIC)
LDFLAGS+=-L$(APPLICATION_PATH)/disp_init/lib -L/usr/local/lib
STDLIBS:=  -lstdc++ -ldl  -lpthread -lm 
LIBS:=-Wl,-Bdynamic ${MINIGUI_LIBS} ${PLAT_DEPENDENT_LIB_EX} ${PLAT_DEPENDENT_LIB} -Wl,-Bdynamic $(STDLIBS)
CFLAGS:=-rdynamic -funwind-tables -I. -I$(SDK_INCS) -I$(APPLICATION_PATH)/disp_init/inc -I$(UAPI_INCS)
#-I$(PWD)/library/include -I$(PWD)/library/include/ctrl -I$(PWD)/library/include/libpng16/ -I/usr/local/include -I$(UAPI_INCS)
CC:=arm-linux-gnueabihf-gcc


LOGO_SRC:=$(APPLICATION_PATH)/disp_init/src/disp_init.c  \
		$(APPLICATION_PATH)/disp_init/src/sstardisp.c \
		$(APPLICATION_PATH)/disp_init/src/bmp.c  \
		$(APPLICATION_PATH)/disp_init/src/jpeg.c
LOGO:=disp_init

.PHONY : app_src app_clean


app_src:
	$(CC) -D_GNU_SOURCE -o  $(APPLICATION_PATH)/disp_init/src/$(LOGO)  $(LOGO_SRC) $(CFLAGS) $(LDFLAGS) $(LIBS) 
app_clean:
	rm -f $(patsubst %.c,%.o,$(LOGO_SRC))
	rm -f $(APPLICATION_PATH)/disp_init/src/$(LOGO)
install:
	@echo done!

