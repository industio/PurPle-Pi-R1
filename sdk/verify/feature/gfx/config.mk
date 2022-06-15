include clear-config.mk
DEP_MODULE:common sys
CFLAGS:=-O0
#SRCS:=common/verify_gfx.c
#LIBS:=mi_sys mi_gfx
COMPILE_TARGET:=nop
include add-config.mk
