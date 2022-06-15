include clear-config.mk
DEP_MODULE:common sys
CFLAGS:=-O3 -Ofast -ftree-vectorize -mfpu=neon
SRCS:=tc_rotate.c ../common/verify_gfx.c ../common/blitutil.c
LIBS:=mi_sys mi_gfx
include add-config.mk
