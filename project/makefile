.PHONY: release image
include configs/current.configs

symbol_link: kbuild/$(KERNEL_VERSION)/.config

symbol_link_clean:
	$(MAKE) -f kbuild/$(KERNEL_VERSION)/kbuild.mk symbol_link_clean

kbuild/$(KERNEL_VERSION)/.config:
	$(MAKE) -f kbuild/$(KERNEL_VERSION)/kbuild.mk symbol_link

-include $(SOURCE_MK)

release: $(SOURCE_RELEASE)
	@echo $(SOURCE_RELEASE)

image-ota:
	$(MAKE) -C image ota_images

image_all: image_clean
	$(MAKE) -C image all
	./split_partion.sh
image: symbol_link release error_message image_all

image-nocheck: symbol_link release image_all warning_message

image-fast: error_message image_all

image-fast-nocheck: image_all warning_message

warning_message:
	@if [ -d ".git" ]; then \
		cd  release/include && \
		git status -s . | \
		awk -F' ' '{a = a "\033[7;34;42m""release/include/"$$2"\033[0m\n"} END {if(a){ \
				print "\033[5;41;33mWARNING!!!! You CAN NOT MODIFY HEAD FILES ,NEED MI OWNER TO REVIEW THE FOLLOWING INCONSISTENT HEAD FILES!!!\033[0m"; \
				print "\033[5;41;33mIF YOU WANT DEBUG ONLY, pelase using command :[[[ make image-nocheck ]]] INSTEADED!!!\033[0m"; \
				print a;}}'; \
	fi;

error_message:warning_message
	@if [ -d ".git" ]; then \
		cd  release/include && \
		git status -s . | \
		awk -F' ' '{a = a "\033[7;34;42m""release/include/"$$2"\033[0m\n"} END {if(a){ \
				exit 127;}}'; \
	fi;

image_clean:
	$(MAKE) -C image clean


distclean: $(SOURCE_CLEAN) image_clean symbol_link_clean

clean:
	@echo "CLEAN_KEEP_RELEASE = 0" >> configs/current.configs
	make distclean
	sed -i '/CLEAN_KEEP_RELEASE/d' configs/current.configs

clean_keep_release:
	@echo "CLEAN_KEEP_RELEASE = 1" >> configs/current.configs
	make distclean
	sed -i '/CLEAN_KEEP_RELEASE/d' configs/current.configs

build_verify: $(SOURCE_RELEASE_DEPEND) clean image
