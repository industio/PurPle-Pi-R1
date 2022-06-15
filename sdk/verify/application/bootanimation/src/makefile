PWD=`pwd`
PROJ_ROOT:=/home/industio/ssd20x/project
APPLICATION_PATH:=/home/industio/ssd20x/sdk/verify/application
SDK_LIBS=$(PROJ_ROOT)/release/nvr/i2m/common/glibc/8.2.1/mi_libs/dynamic
SDK_LIBS_STATIC=$(patsubst %dynamic,%static,$(SDK_LIBS))
PLAT_DEPENDENT_LIB=-lmi_gfx -lmi_sys -lmi_common -ldl 
PLAT_DEPENDENT_LIB_EX= -lmi_disp -lmi_panel -lmi_vdec

SDK_INCS=$(PROJ_ROOT)/release/include
UAPI_INCS=$(PROJ_ROOT)/kbuild/4.9.84/i2m/include/uapi/mstar
LDFLAGS+=-L$(SDK_LIBS)  -L$(SDK_LIBS_STATIC)

STDLIBS:=  -ldl  -lpthread -lm 
LIBS:=-Wl,-Bdynamic  ${PLAT_DEPENDENT_LIB_EX} ${PLAT_DEPENDENT_LIB} -Wl,-Bdynamic $(STDLIBS)
CFLAGS:=-rdynamic -funwind-tables -I. -I$(SDK_INCS) -I$(UAPI_INCS)
#-I$(PWD)/library/include -I$(PWD)/library/include/ctrl -I$(PWD)/library/include/libpng16/ -I/usr/local/include -I$(UAPI_INCS)
CC:=arm-linux-gnueabihf-gcc


LOGO_SRC:=$(APPLICATION_PATH)/bootanimation/src/bootanimation.c 
 
LOGO:=bootanimation

.PHONY : app_src app_clean


app_src:
	$(CC) -D_GNU_SOURCE -o  $(APPLICATION_PATH)/bootanimation/src/$(LOGO)  $(LOGO_SRC) $(CFLAGS) $(LDFLAGS) $(LIBS) 
app_clean:
	rm -f $(patsubst %.c,%.o,$(LOGO_SRC))
	rm -f $(APPLICATION_PATH)/jpeg2disp/src/$(LOGO)
install:
	@echo done!
