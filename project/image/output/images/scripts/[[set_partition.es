# <- this is for comment / total file size must be less than 4KB
mtdparts del CIS
setenv mtdparts $(mtdparts),0x20000(KEY_CUST),0x60000(LOGO),0x500000(KERNEL),0x500000(RECOVERY),-(UBI)
saveenv
nand erase.part UBI
ubi part UBI
ubi create rootfs 0x5200000
 ubi create miservice 0xA00000
 ubi create customer 0x500000
 ubi create appconfigs 0x400000

% <- this is end of file symbol
