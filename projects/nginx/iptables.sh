#! /bin/bash

#start ip forward
echo 1 > /proc/sys/net/ipv4/ip_forward
#echo "1" > /proc/sys/net/bridge/bridge-nf-call-iptables


#drop nat
iptables -t nat -F
iptables -F

#configuration nat
# --src 192.168.30.0/0

iptables -A INPUT -d 10.0.0.0/8 -j DROP
iptables -A OUTPUT -d 10.0.0.0/8 -j DROP

# don't disturb
iptables -t nat -d 10.0.0.0/8 -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-port 10001

iptables -t nat -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-port 8080
iptables -t nat -A PREROUTING -p tcp -m tcp --dport 443 -j REDIRECT --to-port 443

iptables -L -n
iptables -t nat -L -n
