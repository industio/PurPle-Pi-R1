INPUT_JPG=./alinsa.jpg
OUTPUT_BIN=./out_logo.bin

gcc gen_header.c -o gen_header
rm $OUTPUT_BIN
./gen_header $OUTPUT_BIN -t 50 -w 1920 -h 1080 -a 0x20300000 -s 0x400000
rm gen_header
cat $INPUT_JPG >> $OUTPUT_BIN

#"LOGO" partition size limitation
if [ `stat -c %s $OUTPUT_BIN` -gt $((512*1024)) ]; then
echo "Error: Check LOGO size"
exit 
fi
