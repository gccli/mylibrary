#! /bin/bash

#start ip forward
echo 1 > /proc/sys/net/ipv4/ip_forward
#echo "1" > /proc/sys/net/bridge/bridge-nf-call-iptables

# Interested destination ip address
DEST=140.207.202.226

#drop nat
iptables -t nat -F
iptables -F

# drop packets
#iptables -A INPUT -d 10.0.0.0/8 -j DROP
#iptables -A OUTPUT -d 10.0.0.0/8 -j DROP

#configuration nat

# redirect boring dest ip to fake port
#iptables -t nat -d 10.0.0.0/8 -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-port 10001

# only redirect specific dest ip, e.g. 106.120.160.78 for yunpan
if [ -n "$DEST" ]; then
    iptables -t nat -A PREROUTING -p tcp -m tcp -d $DEST --dport 80 -j REDIRECT --to-port 8080
else
    iptables -t nat -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-port 8080
fi
iptables -t nat -A PREROUTING -p tcp -m tcp --dport 443 -j REDIRECT --to-port 443
iptables -L PREROUTING -t nat -n

# To print all IPv4 HTTP packets to and from port 80,
# i.e. print only packets that contain data, not, for example, SYN and FIN packets and ACK-only packets.
# tcpdump 'tcp port 80 and (((ip[2:2] - ((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)'

# Dump HTTP GET/POST
# tcpdump -nn -i eth2 'tcp port 80 and tcp[((tcp[12]&0xf0)>>2):4]=0x47455420'
tcpdump -nn -i eth2 'tcp port 80 and tcp[((tcp[12]&0xf0)>>2):4]=0x504f5354'
