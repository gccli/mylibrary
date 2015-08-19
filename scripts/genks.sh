#! /bin/bash

ISO=ubuntu-15.04-server-amd64.iso

mkdir -p /mnt/iso
mount -o loop $ISO /mnt/iso

mkdir -p /opt/ubuntuiso
cp -rT /mnt/iso /opt/ubuntuiso

cd /opt/ubuntuiso
echo en >isolinux/lang

apt-get install system-config-kickstart -y
system-config-kickstart --generate ks.cfg



