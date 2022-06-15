.PHONY: all
OUT_PATH := $(shell pwd)/out

all:
	@rm -rf $(OUT_PATH)
	@mkdir -p $(OUT_PATH)
	@tar -vxf zlib-1.2.11.tar.gz -C $(OUT_PATH)
	@export CC=$(TOOLCHAIN_REL)gcc;	\
	export AR=$(TOOLCHAIN_REL)ar;	\
	cd $(OUT_PATH)/zlib-1.2.11/;	\
	./configure --prefix=$(OUT_PATH);	\
	$(MAKE) && $(MAKE) install;	\
	cd -;
	@mkdir -p $(shell pwd)/lib/static/
	@mv $(OUT_PATH)/lib/libz.a $(shell pwd)/lib/static/
	@mkdir -p $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libz.so $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libz.so.1 $(shell pwd)/lib/dynamic/
	@mv $(OUT_PATH)/lib/libz.so.1.2.11 $(shell pwd)/lib/dynamic/
	@cp $(OUT_PATH)/include $(shell pwd)/ -rf
	@rm -rf $(OUT_PATH)
