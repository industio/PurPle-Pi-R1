.PHONY: all
OUT_PATH := $(shell pwd)/out

all:
	rm -rf $(OUT_PATH)
	mkdir -p $(OUT_PATH)
	tar -vxf libnl-3.5.0.tar.gz -C $(OUT_PATH)
	cd $(OUT_PATH)/libnl-3.5.0;	\
	./configure --prefix=$(OUT_PATH)  CC=$(TOOLCHAIN_REL)gcc --enable-shared --enable-static --host=$(shell $(TOOLCHAIN_REL)gcc -dumpmachine);  \
	$(MAKE) && $(MAKE) install;	\
	cd -;
	mkdir -p $(shell pwd)/lib/static
	cp $(OUT_PATH)/lib/*.a $(shell pwd)/lib/static/ -rf
	mkdir -p $(shell pwd)/lib/dynamic
	cp -d -rf $(OUT_PATH)/lib/* $(shell pwd)/lib/dynamic/
	mkdir -p $(shell pwd)/include
	cp -rf $(OUT_PATH)/include $(shell pwd)/include
	rm -rf $(OUT_PATH)
