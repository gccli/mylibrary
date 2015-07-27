#! /bin/bash

virsh shutdown controller 
virsh shutdown network
virsh shutdown compute0
virsh shutdown compute1
