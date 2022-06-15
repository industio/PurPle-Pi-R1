# All rights reserved.
#
# Makefile used for building application.
#
################################################################################
include ../../../project/configs/current.configs
.PHONY : all clean
all:
ifeq ($(CHIP), i6)
	$(MAKE) -C ./hoth all
endif
ifeq ($(CHIP), i6e)
	$(MAKE) -C ./alderaan all
endif
ifeq ($(CHIP), i6b0)
	$(MAKE) -C ./jedi all
endif
ifeq ($(CHIP), i2m)
	$(MAKE) -C ./geonosis all
endif

clean:
ifeq ($(CHIP), i6)
	$(MAKE) -C ./hoth clean
endif
ifeq ($(CHIP), i6e)
	$(MAKE) -C ./alderaan clean
endif
ifeq ($(CHIP), i6b0)
	$(MAKE) -C ./jedi clean
endif
ifeq ($(CHIP), i2m)
	$(MAKE) -C ./geonosis clean
endif
	@rm -rvf ./out -rvf

install:
ifeq ($(CHIP), i6)
	$(MAKE) -C ./hoth install
endif
ifeq ($(CHIP), i6e)
	$(MAKE) -C ./alderaan install
endif
ifeq ($(CHIP), i6b0)
	$(MAKE) -C ./jedi install
endif
ifeq ($(CHIP), i2m)
	$(MAKE) -C ./geonosis install
endif
