INC  += ./
SUBDIRS += ./
LIBS += -lmi_common -lmi_sys -lmi_ai -lm -lmi_ao -ldl

ifdef ENABLE_HMDI
LIBS += -lmi_hdmi
EXTRA_CFLAGS+= -DHDMI=1
endif
