#! /usr/bin/env python

import re
import sys
import getopt
import socket

cls = {}
cls[1]='A'
cls[2]='B'
cls[3]='C'

def check_ipv4(addr):
    if(re.match("(^[2][0-5][0-5]|^[1]{0,1}[0-9]{1,2})\.([0-2][0-5][0-5]|[1]{0,1}[0-9]{1,2})\.([0-2][0-5][0-5]|[1]{0,1}[0-9]{1,2})\.([0-2][0-5][0-5]|[1]{0,1}[0-9]{1,2})$", addr) != None):
        return True

    return False

def ntoa(tmp):
    x=((tmp&0xff000000) >> 24, (tmp&0xff0000) >> 16, (tmp&0xff00) >> 8, (tmp&0xff))
    s='%s.%s.%s.%s' % x

    return s,x

def aton(tmp):
    tmp=tmp.split('.')
    i=(int(tmp[0]) << 24) | (int(tmp[1]) << 16) | (int(tmp[2]) << 8) | int(tmp[3])

    return i

def check_mask(mask):
    imask=aton(mask)

    for i in range(32):
        if (0x80000000 & imask):
            imask = 0xffffffff & (imask << 1)
        else:
            break

    if imask != 0:
        return False

    return True

def get_class(ipaddr):
    iaddr=aton(ipaddr)
    iaddr = iaddr >> 24

    if ((iaddr & 0xc0) >> 6) == 0 or ((iaddr & 0xc0) >> 6) == 1:
        return 1
    elif ((iaddr & 0xc0) >> 6) == 2:
        return 2
    elif ((iaddr & 0xc0) >> 6) == 3:
        return 3
    else :
        return 0

def get_cidr(imask):
    for i in range(32):
        if (0x80000000 & imask):
            imask = 0xffffffff & (imask << 1)
        else:
            break
    return i

# E.g. ip address is 192.168.18.2/27, the CIDR is 27 and the netmask is 255.255.255.224
def get_mask_by_len(cidr):
    stmp='0b'
    suff='0b'
    for j in range(cidr):
        stmp += '1'
    for j in range(32-cidr):
        stmp += '0'
        suff += '1'

    mask=(int(stmp, 2),int(suff,2))
    return mask

def network_partition(ipaddr,bcast,mask):
    iaddr=aton(ipaddr)
    imask=aton(mask)
    ibcast=aton(bcast)

    inet = (imask&ibcast)
    snet,_=ntoa(inet)
    cidr=get_cidr(imask)
    step=pow(2,32-cidr)

    r=0
    netstart=0
    c=get_class(bcast)
    if c==0:
        print 'Not support'
        return 
    elif c==1:
       r=pow(2,24)
       netstart=(0xff000000 & inet)
    elif c==2:
       r=pow(2,16) 
       netstart=(0xffff0000 & inet)
    elif c==3:
       netstart=(0xffffff00 & inet)
       r=pow(2,8) 

    print 'Class %s network : %s/%d - 0x%x' % (cls[c],snet,cidr,inet)
    
    x,y=get_mask_by_len(cidr)

    addr = []
    addr.append('')
    addr.append('')
    print 'Subnet : maximum addresses %d' % step
    for j in xrange(0,r,step):
        subnet,_=ntoa(netstart+j)
        addr[0],_=ntoa(netstart+j+1)
        addr[1],_=ntoa(netstart+j+step-1)
        if (iaddr > netstart+j and iaddr <netstart+j+step-1):
            print '\033[32mnetwork: %-16s bcast: %-16s valid address %-16s - %-16s\033[0m' % (subnet,addr[1],addr[0],addr[1])
        else:
            print 'network: %-16s bcast: %-16s valid address %-16s - %-16s' % (subnet,addr[1],addr[0],addr[1])

def usage():
    print 'ipcalc.py [ -a address ] [ -b bcast ] [ -m mask ] [ --cidr=cidr ]'

def main():
    addr=''
    mask=''
    bcast=''
    cidr=''

    try:
        opts,args = getopt.getopt(sys.argv[1:], "a:b:m:", ["address=", "bcast=", "mask=", "cidr="])
    except getopt.GetoptError as err:
        print str(err)
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-a", "--address"):
            addr=a
        elif o in ("-b", "--bcast"):
            bcast=a
        elif o in ("-m", "--mask"):
            mask=a
        elif o in ("--cidr"):
            cidr=a
        else:
            usage()
            sys.exit(2)

    if cidr:
        tmp=cidr.split('/')
        if len(tmp) != 2:
            return
        addr=tmp[0]
        iaddr=aton(addr)
        cidr=int(tmp[1])
        imask,y=get_mask_by_len(cidr)
        mask,_=ntoa(imask)
        inet=(iaddr&imask)
        c=get_class(addr)
        if (c==0):
            return
        elif (c==1):
            y=(y&0xffffff)
        elif (c==2):
            y=(y&0xffff)
        elif (c==3):
            y=(y&0xff)
        bcast=(inet|y)
        bcast,_=ntoa(bcast)

        print 'address: %s  bcast: %s  mask: %s' % (addr,bcast, mask)
        network_partition(addr,bcast,mask)

        return 

    if not addr:
        addr = raw_input('Input ip address: ')
    if not check_ipv4(addr):
        print 'invalid ipv4 address'
        sys.exit(2)

    if not mask:
        mask = raw_input('Input netmask: ')
    if not check_ipv4(mask) or not check_mask(mask):
        print 'invalid netmask'
        sys.exit(2)

    if not bcast:
        bcast = raw_input('Input broadcast address: ')
    if not check_ipv4(bcast):
        print 'invalid broadcast address'
        sys.exit(2)


    network_partition(addr,bcast, mask)

if __name__ == "__main__":
    main()
