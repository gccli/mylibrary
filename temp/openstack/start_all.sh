#! /bin/bash

function wait_host {
    local host=$1
    while true
    do
	ping -c1 -W1 $host >/dev/null 2>&1 
	[ $? -eq 0 ] && break
	echo -n '.'
    done
    ping -c1 -W1 $host
}

virsh start controller 
wait_host controller

virsh start network
sleep 5
virsh start compute0
sleep 5
virsh start compute1
echo "done"
