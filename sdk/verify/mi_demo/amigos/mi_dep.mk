include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
INTERFACE_MODULES := vpe vif divp disp vdec venc rgn vdisp sys ai ao gfx ipu sensor vdf ldc shadow panel hdmi isp iqserver
INTERFACE_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(INTERFACE_MODULES),$(n)_$(interface_$(n))_))))
INTERFACE_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))

CODEDEFINE += $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n)))
