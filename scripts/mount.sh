#! /bin/bash

# Handy script for my work environment
# mount file server 'centos0' to local machine
# set loop device for cdrom load, and convenient for such as apt-cdrom utilities

mkdir -p /mnt/centos0
mount -t cifs //centos0/root /mnt/centos0
losetup /dev/loop0 /mnt/centos0/temp/iso/ubuntu-13.10-server-amd64.iso

grep cdrom /etc/fstab > /dev/null
if [ $? -ne 0 ]; then
    echo /dev/loop0 /media/cdrom iso9660 ro,user,noauto 0 0 >> /etc/fstab
    cat /etc/fstab
fi
