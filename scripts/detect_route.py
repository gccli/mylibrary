#! /usr/bin/env python
import re
from subprocess import *
from pprint import pprint as pp

def get_route_info():
  command='/sbin/route -n'
  command=command.split(' ')
  p = Popen(command,stdout=PIPE)
  o = p.communicate()[0]
  m = re.search('.*UG', o)
  if m:
      line=m.group(0)
      l=re.split('[\s]+', line)
      return l[1]

  return ''

def get_local_ip_address(target):
  ipaddr = ''
  if target:
      command='/sbin/ifconfig %s' % target
  else:
      command='/sbin/ifconfig'
  command=command.split(' ')
  p = Popen(command,stdout=PIPE)
  output = p.communicate()[0]

  netif=''
  addrinfo={}
  for line in output.split('\n'):
      if netif:
          addrinfo[netif] = re.split('[:\sa-zA-Z]+', line)
          addrinfo[netif].pop(0)
          netif=''
      m=re.search('^eth[0-9:]*\s+', line)
      if m:
          netif=m.group(0).strip()
      else:
          netif=''

  return addrinfo

def ping(host,timeout=None):
    if not timeout:
        command='ping -W 1 %s -c 3' % host
    else:
        command='ping -W %d %s -c 3' % (timeout,host)

    command=command.split(' ')
    p = Popen(command,stdout=PIPE)
    o = p.communicate()[0]
    if p.returncode != 0:
        print 'Host %s seem unreachable' % gateway
        print o
        return False

    return True


def replace_gateway(oldgw,newgw):
    try:
        command='/sbin/route del default gw %s' % oldgw
        command=command.split(' ')
        p = Popen(command,stdout=PIPE)
        o = p.communicate()[0]
        if p.returncode != 0:
            print o
            return False
    except OSError as e:
        print o
        return False


    command='sed -ibak /%s/d /etc/resolv.conf' % oldgw
    print command
    command=command.split(' ')
    p = Popen(command,stdout=PIPE)

    command='/sbin/route add default gw %s dev eth0' % newgw
    command=command.split(' ')
    p = Popen(command,stdout=PIPE)
    o = p.communicate()[0]
    if p.returncode != 0:
        print o
        return False

    return True

if __name__ == '__main__':
    addresses=get_local_ip_address(None)
    print 'Address info:'
    for name,info in addresses.items():
        print '  %-8s address: %-16s netmask %s' % (name,info[0],info[2])

    gw=get_route_info()
    if not ping(gw):
        s = raw_input('Input new gateway: ')
        s=s.strip()
        if s=='':
            sys.exit(1)
        replace_gateway(gw,s)

        gw=get_route_info()
        ping('www.baidu.com',5)
    else:
        if ping('www.baidu.com',5):
            print 'Gateway:%s Network is OK' % gw
