include $(APPLICATION_PATH)/jpeg2disp/src/makefile
app:app_src
ifeq ($(IMAGE_INSTALL_DIR),)
	@echo "directory of image is not defined"
	@exit 1
endif
	@cp -rf $(APPLICATION_PATH)/jpeg2disp/lib $(IMAGE_INSTALL_DIR)/customer
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(IMAGE_INSTALL_DIR)/customer/lib/*
ifeq ($(PROJECT), xiangfan)
	@cp -rf $(APPLICATION_PATH)/jpeg2disp/res/xiangfan480x480.jpg $(IMAGE_INSTALL_DIR)/customer/logo.jpg
	@cat $(APPLICATION_PATH)/jpeg2disp/run_xiangfan.sh >> $(IMAGE_INSTALL_DIR)/customer/demo.sh
else
	@cp -rf $(APPLICATION_PATH)/jpeg2disp/res/logo1024x600.jpeg $(IMAGE_INSTALL_DIR)/customer/logo.jpg
	@cat $(APPLICATION_PATH)/jpeg2disp/run_raw.sh >> $(IMAGE_INSTALL_DIR)/customer/demo.sh
endif
	@cp -rf $(APPLICATION_PATH)/jpeg2disp/src/logo $(IMAGE_INSTALL_DIR)/customer
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(IMAGE_INSTALL_DIR)/customer/logo
	@make app_clean

