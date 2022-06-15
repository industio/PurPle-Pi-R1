APPLICATION_PATH=$(PROJ_ROOT)/../sdk/verify/application
include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
ifeq ($(verify_smarttalk),enable)
	include $(APPLICATION_PATH)/smarttalk/image.mk
endif
ifeq ($(verify_ssplayer),enable)
	include $(APPLICATION_PATH)/ssplayer/image.mk
endif
ifeq ($(verify_smarthome),enable)
	include $(APPLICATION_PATH)/smarthome/image.mk
endif
ifeq ($(verify_zk_bootup),enable)
	include $(APPLICATION_PATH)/zk_bootup/image.mk
endif
ifeq ($(verify_zk_mini_security),enable)
	include $(APPLICATION_PATH)/zk_mini_security/image.mk
endif
ifeq ($(verify_zk_mini_security_fastboot),enable)
	include $(APPLICATION_PATH)/zk_mini_security_fastboot/image.mk
endif
ifeq ($(verify_jpeg2disp),enable)
	include $(APPLICATION_PATH)/jpeg2disp/image.mk
endif
ifeq ($(verify_zk_full_security),enable)
	include $(APPLICATION_PATH)/zk_full_security/image.mk
endif
ifeq ($(verify_zk_full_security_fastboot),enable)
	include $(APPLICATION_PATH)/zk_full_security_fastboot/image.mk
endif
ifeq ($(verify_bootanimation),enable)
	include $(APPLICATION_PATH)/bootanimation/image.mk
endif

ifeq ($(verify_disp_init),enable)
ifeq ($(PROJECT),2D07)
	#cp -f $(APPLICATION_PATH)/disp_init/src/sstardisp_2D07.c $(APPLICATION_PATH)/disp_init/src/sstardisp.c 
	include $(APPLICATION_PATH)/disp_init/image.mk
endif
ifeq ($(PROJECT),xiangfan)
	#cp -f $(APPLICATION_PATH)/disp_init/src/sstardisp_xiangfan.c $(APPLICATION_PATH)/disp_init/src/sstardisp.c 
	include $(APPLICATION_PATH)/disp_init/image_xiangfan.mk
endif
ifeq ($(PROJECT),2D06)
	#cp -f $(APPLICATION_PATH)/disp_init/src/sstardisp_2D06.c   $(APPLICATION_PATH)/disp_init/src/sstardisp.c 
	include $(APPLICATION_PATH)/disp_init/image.mk
endif
endif

