# <- this is for comment / total file size must be less than 4KB
ubi part UBI
tftp 0x21000000 customer.ubifs
ubi write 0x21000000 customer ${filesize}
% <- this is end of file symbol
