#!/usr/bin/python

import os, sys, struct
import binascii

HEADER_VER = 1
MAGIC_ID = 0x5346574C
ALIGN_UNIT = 131072
PARTITION_HEADER_SIZE = 64
FILE_HEADER_SIZE = 64
COMPRESS_TYPE = 0

def calc_crc32(filename):
    buf = open(filename,'rb').read()
    buf = (binascii.crc32(buf) & 0xFFFFFFFF)
    return buf

def main():
    if len(sys.argv) != 3:
        print 'Input error! usage:\n    %s FOLDER_NAME OUTPUT_BIN_NAME' %sys.argv[0]
        return

    ALIGN_UNIT_cnt = 0;
    write_ptr = 0

    entries = os.listdir(sys.argv[1])
    
    header_size = len(entries)*FILE_HEADER_SIZE + PARTITION_HEADER_SIZE
    """print 'Partition Header:'
    print '    magic id    : 0x%08X' %MAGIC_ID
    print '    hearer ver  : 0x%08X' %HEADER_VER
    print '    header size : %d' %header_size
    print '    file number : %d' %len(entries)"""
    write_ptr += ((header_size-1)/ALIGN_UNIT+1)*ALIGN_UNIT;
    
    image_name = sys.argv[2]
    # Write partition header
    image = open(image_name, 'wb')
    image.write(struct.pack('I', MAGIC_ID))             # magic id
    image.write(struct.pack('I', HEADER_VER))           # header version
    image.write(struct.pack('I', header_size))          # header size
    image.write(struct.pack('I', ALIGN_UNIT))           # access align unit
    image.write(struct.pack('I', len(entries)))         # file number
    # Padding partition header to PARTITION_HEADER_SIZE
    for x in range(PARTITION_HEADER_SIZE-image.tell()%PARTITION_HEADER_SIZE):
        image.write('%c' % 0x00)
    
    for entry in entries:
        
        file_path = sys.argv[1] + "/" + entry
        file_len = os.path.getsize(file_path)
        crc32_value = calc_crc32(file_path)
        """print 'File Header:'
        print '    file name   : %s' %file_path
        print '    position    : 0x%08X' %write_ptr
        print '    file len    : %d' %file_len
        print '    padding len : %d' %(ALIGN_UNIT - file_len%ALIGN_UNIT)
        print '    crc32       : 0x%08X' %crc32_value
        print '    compress    : 0x%08X' %COMPRESS_TYPE"""
        
        # Write file header
        image.write(struct.pack('32s', entry))          # file name
        image.write(struct.pack('I', write_ptr))        # file position
        image.write(struct.pack('I', file_len))         # file length
        image.write(struct.pack('I', (ALIGN_UNIT - file_len%ALIGN_UNIT)))   # padding length
        image.write(struct.pack('I', crc32_value))      # crc32
        image.write(struct.pack('I', COMPRESS_TYPE))    # compress type
        # Padding file header to FILE_HEADER_SIZE
        for x in range(PARTITION_HEADER_SIZE-image.tell()%PARTITION_HEADER_SIZE):
            image.write('%c' % 0x00)
        write_ptr += ((file_len-1)/ALIGN_UNIT+1)*ALIGN_UNIT
    
    # Padding header
    for x in range(((header_size-1)/ALIGN_UNIT+1)*ALIGN_UNIT-image.tell()):
        image.write('%c' % 0x00)
    
    # Write file content
    for entry in entries:
        file_path = sys.argv[1] + "/" + entry
        f = open(file_path, "r")
        image.write(f.read())
        # Padding file content
        for x in range(ALIGN_UNIT-image.tell()%ALIGN_UNIT):
            image.write('%c' % 0x00)
    
    image.close()

    print '    %d files packed in %s' %(len(entries), image_name)

if __name__ == '__main__':
    main()