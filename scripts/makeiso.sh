#! /bin/bash

# http://askubuntu.com/questions/122505/how-do-i-create-a-completely-unattended-install-of-ubuntu
# https://help.ubuntu.com/lts/installation-guide/i386/ch04s06.html


cd /opt/ubuntuiso

apt-get -y install genisoimage


echo 'd-i partman/confirm_write_new_label boolean true
d-i partman/choose_partition select Finish partitioning and write changes to disk
d-i partman/confirm boolean true' > ks.preseed

echo 'label install
  menu label ^Install Ubuntu Server
  kernel /install/vmlinuz
  append file=/cdrom/preseed/ubuntu-server.seed initrd=/install/initrd.gz ks=cdrom:/ks.cfg preseed/file=/cdrom/ks.preseed ---' > isolinux/txt.cfg

mkisofs -D -r -V "ATTENDLESS_UBUNTU" -cache-inodes -J -l -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o ~/autoinstall.iso /opt/ubuntuiso
