#! /bin/bash

echo -e "root\tsoft\tnofile\t16384" | tee -a /etc/security/limits.conf
echo -e "root\thard\tnofile\t16384" | tee -a /etc/security/limits.conf
