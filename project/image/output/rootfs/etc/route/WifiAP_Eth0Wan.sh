#!/bin/sh
/config/wifi/ssw01bInit.sh
sleep 1
ifconfig wlan0 192.168.3.2 up
hostapd -d /etc/hostapd.conf &
sleep 5
udhcpd -S /etc/udhcpd_wlan0.conf
sleep 3
echo "1" > /proc/sys/net/ipv4/ip_forward
iptables -F
iptables -t nat -F
iptables -t mangle -F
iptables -X
iptables -t nat -X
iptables -t mangle -X
iptables -P INPUT ACCEPT
iptables -P OUTPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -t nat -A POSTROUTING -j MASQUERADE
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

