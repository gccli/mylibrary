#! /bin/bash
#
#
# variables

PUBIF=eth0
PUBIP=$(ifconfig $PUBIF |sed -ne 's/.*inet addr:\([0-9.]*\) .*/\1/p')
PRIIF=eth1
PRIIP=$(ifconfig $PRIIF |sed -ne 's/.*inet addr:\([0-9.]*\) .*/\1/p')
PUBNET=$(ip route show | grep $PUBIP | awk '{ print $1 }')
PRINET=$(ip route show | grep $PRIIP | awk '{ print $1 }')

ACTION=$1
TABLE=$2
[ -z "$ACTION" ] && ACTION=help
[ -z "$TABLE" ]  && TABLE=filter

function usage() {
    echo -e "Usage: iptables.sh [ ACTION ] [ TABLE ]"
    echo -e "       ACTION can be help, show, on, off"
    echo -e "       TABLE can be filter, nat"
}

function setup_filter() {
    iptables
}

function setup_nat() {
    echo 1 > /proc/sys/net/ipv4/ip_forward

    # SNAT
    iptables -t nat -A POSTROUTING -s $PRINET -o $PUBIF -j SNAT --to-source $PUBIP
    iptables -t nat -A POSTROUTING -s $PUBNET -o $PRIIF -j SNAT --to-source $PRIIP
    # DNAT
    #iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 80 -j DNAT --to-source PRI_IP:PRIVATE_POR
    #iptables -t nat -A PREROUTING -p tcp -i $PUBIF --dport 80 -j REDIRECT --to-ports 80
}

function stop_filter() {
    iptables -F 
    iptables -X
    iptables -Z
}

function stop_nat() {
    echo 0 > /proc/sys/net/ipv4/ip_forward
    iptables -t nat -F 
    iptables -t nat -Z
}

function show() {
    iptables -t $TABLE -nv -L
}

case $ACTION in
    help)
	usage
	;;
    show)
	show
	;;
    on)
	if [ $TABLE == "filter" ]; then
	    setup_filter
	elif [ $TABLE == "nat" ]; then
	    setup_nat
	fi
	;;
    off)
	if [ $TABLE == "filter" ]; then
	    stop_filter
	elif [ $TABLE == "nat" ]; then
	    stop_nat
	fi
	;;
esac
