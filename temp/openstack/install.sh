#! /bin/bash


name=controller
path=/dev/openstack/$name
cdrom="/opt/openstack/iso/ubuntu-14.04.2-server-amd64.iso"
network="network=management"
ram=2048


virsh destroy $name 2>/dev/null; virsh undefine $name 2>/dev/null

lvremove -f /dev/openstack/controller /dev/openstack/network /dev/openstack/compute0 /dev/openstack/compute1
lvcreate -n controller -L 100G /dev/openstack
lvcreate -n network -L 100G /dev/openstack
lvcreate -n compute0 -L 100G /dev/openstack
lvcreate -n compute1 -L 100G /dev/openstack


echo virt-install --name $name --ram $ram --disk path=$path --network $network --virt-type qemu --accelerate --vnc --noautoconsole -v --cdrom $cdrom --os-type=linux --os-variant ubuntutrusty
virt-install --name $name --ram $ram --disk path=$path --network $network --virt-type qemu --accelerate --vnc --noautoconsole -v --cdrom $cdrom --os-type=linux --os-variant ubuntutrusty
