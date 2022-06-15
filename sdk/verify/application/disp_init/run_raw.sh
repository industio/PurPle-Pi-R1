export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/customer/lib:/lib
cd /customer
chmod 777 disp_init
# enable LCD pwoer
echo 18 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio18/direction
echo 1 > /sys/class/gpio/gpio18/value
./disp_init &

