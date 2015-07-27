#! /bin/bash
PUBIF=em1
PRI_IP=192.168.10.2

# http://docs.openstack.org/juno/config-reference/content/firewalls-default-ports.html

iptables -F
iptables -t nat -F

iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 80 -j DNAT --to $PRI_IP:80
iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 6080 -j DNAT --to $PRI_IP:6080
iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 6081 -j DNAT --to $PRI_IP:6081
iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 6082 -j DNAT --to $PRI_IP:6082

iptables -A FORWARD -p tcp -d $PRI_IP --dport 6080:6082 -m state --state NEW,ESTABLISHED,RELATED -j ACCEPT

iptables -L -n
echo -e "\n********************************\n"
iptables -t nat -L -n
