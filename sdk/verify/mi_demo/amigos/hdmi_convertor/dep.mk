include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)

DEP += base tem iniparser onvif live555

ifneq ($(interface_rgn), disable)
DEP += rgn
endif

#3RD_PARTY_DEP += z png jpeg