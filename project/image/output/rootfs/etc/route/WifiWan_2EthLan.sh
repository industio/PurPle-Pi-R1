#!/bin/sh
eth0_mac=`ifconfig eth0 | grep HWaddr | awk '{print $5}'`
if [ "$eth0_mac" == "00:30:1B:BA:02:DB" ]; then
        ifconfig eth0 hw ether 88:A9:A7:00:12:34
fi

eth1_mac=`ifconfig eth1 | grep HWaddr | awk '{print $5}'`
if [ "$eth1_mac" == "00:30:1B:BA:02:DB" ]; then
        ifconfig eth1 hw ether 88:A9:A7:00:12:35
fi

mkdir -p /run/network
echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -t nat -F
iptables -t nat -X
iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE
ip link add br-lan type bridge
ip link set eth0 master br-lan
ip link set eth1 master br-lan
ifconfig eth0 0.0.0.0 up
ifconfig eth1 0.0.0.0 up
ifconfig br-lan 192.168.3.1 netmask 255.255.255.0 up
/etc/init.d/S80dhcp-server start

/config/wifi/ssw01bInit.sh
sleep 1
wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant.conf -B &
sleep 10
wlan_gw=`route -n | grep wlan0 | awk 'NR==1{print $2}'`
route add default gw $wlan_gw

