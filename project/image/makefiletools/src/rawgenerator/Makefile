.PHONY: all

TARGET_DISP := dispcfggen
TARGET_LOGO := logogen


all:
	gcc -m32 -Wall ss_raw_header.c logo_data_main.c -o ../../bin/$(TARGET_LOGO)
	gcc -m32 -Wall -I./pnl ss_raw_header.c disp_data_main.c -o ../../bin/$(TARGET_DISP)

	