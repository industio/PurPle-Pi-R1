.PHONY: all
OUT_PATH := $(shell pwd)/out

all:
	@rm -rf $(OUT_PATH)
	@mkdir -p $(OUT_PATH)
	@tar -vxf libpng-1.6.37.tar.gz -C $(OUT_PATH)
	@cd $(OUT_PATH)/libpng-1.6.37;	\
	./configure --prefix=$(OUT_PATH) CC=$(TOOLCHAIN_REL)gcc --host=$(shell $(TOOLCHAIN_REL)gcc -dumpmachine) LIBS=-L$(shell pwd)/../z/lib/dynamic/ CPPFLAGS=-I$(shell pwd)/../z/include/;	\
	$(MAKE) && $(MAKE) install;	\
	cd -;
	@mkdir -p $(shell pwd)/lib/static/
	@mv $(OUT_PATH)/lib/libpng16.a $(shell pwd)/lib/static/
	@mv $(OUT_PATH)/lib/libpng.a $(shell pwd)/lib/static/
	@mkdir -p $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libpng.so $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libpng16.so $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libpng16.so.16 $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libpng16.so.16.37.0 $(shell pwd)/lib/dynamic/
	@cp $(OUT_PATH)/include $(shell pwd)/ -rf
	@rm -rf $(OUT_PATH)