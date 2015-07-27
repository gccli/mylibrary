#! /usr/bin/env python
# -*- coding: latin-1 -*-

import os
import sys
import re
import time
import select
import socket
import dnslib
import pygeoip
import OpenSSL
import netaddr
import random
import threading
import thread
import getopt
import Queue

from proxystatistics import GoogleDB

# dig @8.8.4.4 _netblocks.google.com TXT
# https://support.google.com/a/answer/60764?hl=en
def get_google_ipranges(dnsserver = '8.8.4.4'):
    query = '_netblocks.google.com'
    timeout=10
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ipranges=[]
    ipset=netaddr.IPSet()
    try:
        query = dnslib.DNSRecord(q=dnslib.DNSQuestion(query,dnslib.QTYPE.TXT))
        query_data = query.pack()
        sock.sendto(query_data, (dnsserver, 53))
        ins, _, _ = select.select([sock], [], [], timeout)
        for sock in ins:
            reply_data, reply_address = sock.recvfrom(512)
            reply_server = reply_address[0]
            record = dnslib.DNSRecord.parse(reply_data)
            # print record.rr[0].rdata
            ipranges = [ netaddr.IPNetwork(x) for x in  re.findall(r'[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+/[0-9]+',str(record.rr[0].rdata)) ]
            ipset=netaddr.IPSet(ipranges)

    except socket.error as e:
        print 'handle dns query=%s socket: %r', query, e
        raise socket.gaierror(11004, 'getaddrinfo %r from %r failed' % (query, dnsserver))
    finally:
        sock.close()
    return (ipranges,ipset)

# Google Root CA file
# https://pki.google.com/faq.html
# wget https://pki.google.com/roots.pem
class SSLContext(object):
    ctx = OpenSSL.SSL.Context(OpenSSL.SSL.TLSv1_METHOD)
    def __init__(self):
        self.ctx.set_verify(OpenSSL.SSL.VERIFY_PEER, self.verify_cb)
        self.ctx.load_verify_locations('roots.pem')

    def verify_cb(self, conn, cert, errnum, depth, ok):
        if not ok:
            print 'Got certificate errnum:%d depth:%d ok:%d' % (errnum,depth,ok)
            print '   Issuer  : %s(%s)' %(cert.get_issuer().commonName,cert.get_serial_number())
            print '   Subject :', cert.get_subject().commonName      
        return ok

class SSLConnection(object):
    def __init__(self,context,timeout=3):
        self.connect_timeout = timeout    
        self._context = context
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        self._sock.settimeout(self.connect_timeout)
        self._connection = OpenSSL.SSL.Connection(context, self._sock)
        self._connection.set_connect_state()
        server_hostname = b'www.googleapis.com'
        self._connection.set_tlsext_host_name(server_hostname)
    
    def __del__(self):
        if self._connection:
            self._connection.shutdown()
            self._connection.close()
        if self._sock:
            self._sock.close()

    def __getattr__(self, attr):        
        if attr not in ('_context', '_sock', '_connection'):
            return getattr(self._connection, attr)
    
    def __iowait(self, io_func, *args, **kwargs):
        timeout = self._sock.gettimeout() or 0.1        
        fd = self._sock.fileno()
        err = False
        while True:
            try:
                return io_func(*args, **kwargs)
            except (OpenSSL.SSL.WantReadError, OpenSSL.SSL.WantX509LookupError):
                sys.exc_clear()
                _, _, errors = select.select([fd], [], [fd], timeout)
                if errors:
                    err = True
                    break
            except OpenSSL.SSL.WantWriteError:
                sys.exc_clear()
                _, _, errors = select.select([], [fd], [fd], timeout)
                if errors:
                    err = True
                    break
            if err:
                print 'error occur in iowait'
    
    def do_connect(self, *args, **kwargs):
        return self.__iowait(self._connection.connect, *args, **kwargs)

    def do_handshake(self):
        return self.__iowait(self._connection.do_handshake)
    
    def connect(self, ip):
        addr = (ip, 443)
        try:
            start = time.time()
            self.do_connect(addr)
            connected_time=time.time()-start
            self.do_handshake()
            handshaked_time=time.time()-start                      
            cert = self._connection.get_peer_certificate()
            
            result = {'connect':connected_time, 'handshake':handshaked_time, 'certificate':cert}            
            return result
        except:
            return None

ssl_context = SSLContext()
geo_ip = pygeoip.GeoIP('GeoIP.dat')
google_db = GoogleDB()

def ip_connectivity(iplist,queue):
    #print 'Thread started test ip',iplist
    for ip in iplist:
        #print 'Test ip %s connectivity' % ip
        ssl_conn = SSLConnection(ssl_context.ctx)
        result = ssl_conn.connect(ip)
        if result:
            result['ip'] = ip
            result['region']=geo_ip.country_code_by_addr(ip)
            queue.put(result)
        del ssl_conn

def usage(prog):
    print 'Usage: %s [ -n step ] [ --sample=N|--percent=N ] ip...'
    sys.exit(0)

if __name__ == '__main__':
    _percent=0
    _sample=0  # number of ip address from google ip blocks
    _step=1    # number of ip test by per-thread
    opts, args = getopt.getopt(sys.argv[1:], "n:", ['sample=', 'percent='])
    for o, a in opts:
        if o == "-n":
            _step=int(a)
        elif o == '--sample':
            _sample=int(a)
        elif o == '--percent':
            _percent=int(a)
        else:
            usage()

    _dnsserver='223.6.6.6'

    _randiplist=[]
    _testiplist=list(set(args))
    ipranges,ipset=get_google_ipranges(_dnsserver)
    _range_size=len(ipranges)
    for j in range(_range_size):
        print '[%d] %-20s with %d address' %(j,ipranges[j],ipranges[j].size)
    i=int(raw_input('Select 0-{0} > '.format(_range_size-1)) or '0')
    validate = lambda i: i >=0 and i < _range_size
    if validate(i):
        if _percent:
            _sample=ipranges[i].size*_percent/100
        _randiplist+=random.sample(ipranges[i],_sample)
    else:
        if _percent:
            _sample=ipset[i].size*_percent/100
        _randiplist+=random.sample(ipset,_sample)
    results=[]
    iplist= _testiplist + [ str(x) for x in _randiplist ]
    ipsize= len(iplist)

    _output='Query DNS server %s Number of IP block: %d\n' % (_dnsserver,len(ipranges))
    for x in ipranges:
        _output += '%s %-20s with %d address\n' %('*' if validate(i) and ipranges[i]==x else ' ',x,x.size)
    _output+='Total test ip address: %d (user:%d random:%d)\n' %(len(iplist),len(_testiplist),len(_randiplist))
    print _output

    if (len(iplist)/_step > 100):
        _step=1+len(iplist)/100

    google_db.delete_ip()
    result_queue=Queue.Queue()
    for i in xrange(0,ipsize,_step):
        thread.start_new_thread(ip_connectivity, (iplist[i:i+_step-1], result_queue))

    print '%-20s code connect handshake %-20s %s' % ('address', 'subject', 'issuer')
    goodip = 0
    while True:
        try:
            r=result_queue.get(timeout=10)
            goodip += 1
            cn=r['certificate'].get_subject().commonName
            issuer='%s(%s)' % (r['certificate'].get_issuer().commonName,r['certificate'].get_serial_number())
            print '%-20s %-4s %7.3f %9.6f %-20s %s' % (r['ip'], r['region'], r['connect'],r['handshake'],cn,issuer)
            flags=0
            if r['ip'] in _testiplist:
                flags=1
                if r['ip'] in ipset:
                    flags=3
            google_db.insert_ip(r['ip'],r['region'],r['connect'],r['handshake'],cn,flags)
        except Queue.Empty as err:
            print 'Done, got %d good ip. %d thread still actived' % (goodip, threading.active_count())
            break
    google_db.dump_ip()

