#! /bin/bash

ssh network "echo server controller iburst > /etc/ntp.conf;service ntp restart"
ssh compute0 "echo server controller iburst > /etc/ntp.conf;service ntp restart"
ssh compute1 "echo server controller iburst > /etc/ntp.conf;service ntp restart"
