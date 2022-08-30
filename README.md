# Purple-Pi-R1-sdk
SDK for Purple Pi R1, base on SSD20X V30 SDK.
## build
```
./Release_to_customer.sh -f [nand/nor] -p [ssd201/ssd202] -m [128/256/512] -q [null/fastboot]
example:
 nand flash=256MB, CPU=SSD202D, no select fastboot mode
./Release_to_customer.sh -f nand -p ssd202 -m 256 

 nand flash=128MB, CPU=SSD201, select fastboot mode
./Release_to_customer.sh -f nand -p ssd201 -m 128 -q fastboot
