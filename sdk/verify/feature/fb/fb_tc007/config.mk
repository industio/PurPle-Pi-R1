include clear-config.mk
CFLAGS:=-O0
LIBS:=m
SRCS:=fb_doublebuffer.c
LIBS:=mi_sys mi_gfx

include add-config.mk
