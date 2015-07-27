#! /bin/bash

function netsetup {
    local name=$1
    virsh net-info $name 2>/dev/null
    if [ $? -eq 0 ]; then
	virsh net-destroy $name
	virsh net-undefine $name
    fi
    virsh net-define $name.xml
    virsh net-start $name
    virsh net-autostart $name
}

virsh net-info default 2>/dev/null
if [ $? -eq 0 ]; then
    virsh net-destroy default
    virsh net-undefine default
fi

virsh net-list --all
netsetup management
netsetup tunnel
netsetup storage
virsh net-list --all
