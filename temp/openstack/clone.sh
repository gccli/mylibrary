#! /bin/bash


name=controller
virt-clone -o $name --name network --file /dev/openstack/network
virt-clone -o $name --name compute0 --file /dev/openstack/compute0
virt-clone -o $name --name compute1 --file /dev/openstack/compute1
