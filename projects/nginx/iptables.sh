#! /bin/bash

#start ip forward
echo 1 > /proc/sys/net/ipv4/ip_forward
#echo "1" > /proc/sys/net/bridge/bridge-nf-call-iptables


#drop nat
iptables -t nat -F
iptables -F

#configuration nat
iptables -t nat -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-port 8080
iptables -t nat -A PREROUTING -p tcp -m tcp --dport 443 -j REDIRECT --to-port 443
