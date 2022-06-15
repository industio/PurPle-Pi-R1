include $(APPLICATION_PATH)/disp_init/src/makefile
app:app_src
ifeq ($(IMAGE_INSTALL_DIR),)
	@echo "directory of image is not defined"
	@exit 1
endif
	@cp -rf $(APPLICATION_PATH)/disp_init/lib $(IMAGE_INSTALL_DIR)/customer
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(IMAGE_INSTALL_DIR)/customer/lib/*
	#@cp -rf $(APPLICATION_PATH)/disp_init/res/logo1024x600.jpg $(IMAGE_INSTALL_DIR)/customer/logo.jpg
	@cp -rf $(APPLICATION_PATH)/disp_init/src/disp_init $(IMAGE_INSTALL_DIR)/customer
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(IMAGE_INSTALL_DIR)/customer/disp_init
	@cat $(APPLICATION_PATH)/disp_init/run.sh >> $(IMAGE_INSTALL_DIR)/customer/demo.sh
	@make app_clean

