include $(APPLICATION_PATH)/bootanimation/src/makefile
app:app_src
ifeq ($(IMAGE_INSTALL_DIR),)
	@echo "directory of image is not defined"
	@exit 1
endif
	@cp -rf $(APPLICATION_PATH)/bootanimation/src/bootanimation $(IMAGE_INSTALL_DIR)/customer
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(IMAGE_INSTALL_DIR)/customer/bootanimation
	@$(APPLICATION_PATH)/bootanimation/res/addhead $(APPLICATION_PATH)/bootanimation/res/bootanimation_src.h264 $(APPLICATION_PATH)/bootanimation/res/bootanimation_dst.h264
	@cp -rf $(APPLICATION_PATH)/bootanimation/res/bootanimation_dst.h264 $(IMAGE_INSTALL_DIR)/customer
	@cat $(APPLICATION_PATH)/bootanimation/run.sh >> $(IMAGE_INSTALL_DIR)/customer/demo.sh
	@make app_clean

