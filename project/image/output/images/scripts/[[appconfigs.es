# <- this is for comment / total file size must be less than 4KB
ubi part UBI
tftp 0x21000000 appconfigs.ubifs
ubi write 0x21000000 appconfigs ${filesize}
% <- this is end of file symbol
