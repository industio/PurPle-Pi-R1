TARGET_BOOT_IMAGE:=$(foreach n,$(BOOT_IMAGE_LIST),$(n)_mkboot)

boot_images: $(TARGET_BOOT_IMAGE)

%_mkboot:
	dd if=/dev/zero bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(PATSIZE))) count=1 | tr '\000' '\377' > $(IMAGEDIR)/$@;
	dd if=$($(patsubst %_mkboot,%,$@)$(RESOUCE)) of=$(IMAGEDIR)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(PATSIZE))) count=1 conv=notrunc seek=0;
	if [ "$($(patsubst %_mkboot,%,$@)$(COPIES))" != "" ]; then \
		for((Row=1;Row<$($(patsubst %_mkboot,%,$@)$(COPIES));Row++));do \
			dd if=$($(patsubst %_mkboot,%,$@)$(RESOUCE)) of=$(IMAGEDIR)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(DATASIZE))) count=1 conv=notrunc seek=$${Row};	\
		done; \
	fi;
	if [ "$($(patsubst %_mkboot,%,$@)$(PATCOUNT))" != "" ]; then \
		for((Row=1;Row<$($(patsubst %_mkboot,%,$@)$(PATCOUNT));Row++));do \
			dd if=$(IMAGEDIR)/$@ of=$(IMAGEDIR)/$@ bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(PATSIZE))) count=1 conv=notrunc oflag=append;	\
		done;	\
	fi;
	if [ "$($(patsubst %_mkboot,%,$@)$(BLKSIZE))" != "" ]; then \
		dd if=/dev/zero bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(BLKSIZE))) count=1 | tr '\000' '\377' > $(IMAGEDIR)/$@_blk;	\
		dd if=$(IMAGEDIR)/$@ of=$(IMAGEDIR)/$@_blk bs=$(shell printf "%d" $($(patsubst %_mkboot,%,$@)$(BLKSIZE))) count=1 conv=notrunc seek=0;	\
		mv $(IMAGEDIR)/$@_blk $(IMAGEDIR)/$@;	\
	fi;
	cat $(IMAGEDIR)/$@ >> $(IMAGEDIR)/boot.bin
	rm -rf $(IMAGEDIR)/$@
