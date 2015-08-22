#! /usr/bin/env python

import os
import sys
import pcap
import string
import time
import socket
import struct
import re
import curses
import curses.ascii
import subprocess
import httplib
import tempfile
import logging
import threading
import random

protocols = {socket.IPPROTO_TCP:'tcp', socket.IPPROTO_UDP:'udp', socket.IPPROTO_ICMP:'icmp'}
tcp_flags = {'FIN':0x01,'SYN':0x02,'RST':0x04,'PSH':0x08,'ACK':0x10,'URG':0x20}

def filter_tcp_flags(flags):
    filtered = False
    if flags == tcp_flags['SYN'] or flags == tcp_flags['ACK'] or flags == tcp_flags['FIN'] or flags == (tcp_flags['SYN'] | tcp_flags['ACK']) or flags == (tcp_flags['FIN'] | tcp_flags['ACK']) :
        filtered = True

    return filtered 
    
def decode_tcp_flags(flags):
    s=''
    if flags & 0x01:
        s += 'FIN '
    if flags & 0x02:
        s += 'SYN '
    if flags & 0x04:
        s += 'RST '
    if flags & 0x08:
        s += 'PSH '
    if flags & 0x10:
        s += 'ACK '
    if flags & 0x20:
        s += 'URG '

    s = '(' + s.rstrip(' ') + ')'
    return s

def decode_tcp_packet(s):
    d={}
    d['srcport'] = socket.ntohs(struct.unpack('H',s[0:2])[0])
    d['dstport'] = socket.ntohs(struct.unpack('H',s[2:4])[0])
    d['data_offset'] = (ord(s[12]) & 0xf0) >> 4
    d['flags'] = (ord(s[13]) & 0x3f)
    d['data']=s[4*d['data_offset']:]

    return d

def decode_ip_packet(s):
    d={}
    d['version']=(ord(s[0]) & 0xf0) >> 4
    d['header_len']=ord(s[0]) & 0x0f
    d['tos']=ord(s[1])
    d['total_len']=socket.ntohs(struct.unpack('H',s[2:4])[0])
    d['id']=socket.ntohs(struct.unpack('H',s[4:6])[0])
    d['flags']=(ord(s[6]) & 0xe0) >> 5
    d['fragment_offset']=socket.ntohs(struct.unpack('H',s[6:8])[0] & 0x1f)
    d['ttl']=ord(s[8])
    d['protocol']=ord(s[9])
    d['checksum']=socket.ntohs(struct.unpack('H',s[10:12])[0])
    d['srcaddr']=pcap.ntoa(struct.unpack('i',s[12:16])[0])
    d['dstaddr']=pcap.ntoa(struct.unpack('i',s[16:20])[0])
    if d['header_len']>5:
        d['options']=s[20:4*(d['header_len']-5)]
    else:
        d['options']=None
    d['data']=s[4*d['header_len']:]
    return d


def dumphex(s):
    bytes = map(lambda x: '%.2x' % x, map(ord, s))
    for i in xrange(0,len(bytes)/16):
        print '        %s' % string.join(bytes[i*16:(i+1)*16],' ')
    print '        %s' % string.join(bytes[(i+1)*16:],' ')

def dump_data(s):
    for i in xrange(0,(len(s)+32)/32):
        d=''
        for j in range(32):
            if (i*16+j) == len(s):                
                break
            c=s[i*16+j]
            if j == 16:
                d += '  '
            d += string.join(c if curses.ascii.isprint(c) else '.')
        print d


samples=['samples/AE.Adware', 'samples/ash.dot.gif', 'samples/Bagle-AA', 'samples/mime-rtss', 'samples/rtc', 'samples/rtss']

def scan_by_cca(direction, first_line, header, data):
    '''
    scan pcap file, parse HTTP and call CCA scan program 
    '''
    output = '---------------------------------------- Thread count %d\n' % threading.active_count()

    command_string = './scan -t --oop --traffic-type=1 '
    if (direction == 1):
        headers = {}
        for line in header.split('\r\n'):
            if len(line) > 2 and line.find(':'):
                k,v=line.split(':',1)
                headers[k.strip()] = v.strip()
        method,path,_ = first_line.split(' ')
        host = headers.get('Host')
        if host:
            url='http://%s/%s' % (host,path)
            output += '  Method: %s\n' % method
            output += '  URL: %s\n' % (url if len(url) <= 120 else '{0} (truncated)'.format(url[:120]))

            with tempfile.NamedTemporaryFile(mode='wb',prefix='u.', delete=False) as url_file:
                url_file.write(url)
                url_file.close()
                command_string += '-O '
                command_string += '--url={0} '.format(url_file.name)

        ua=headers.get('User-Agent')
        if ua:
            output += '  User-Agent: %s\n' % (ua if len(ua) <= 120 else '{0} (truncated)'.format(ua[:120]))
    else:
        _,status = first_line.split(' ',1)
        output += '  %s\n' % first_line
        for line in header.split('\r\n'):
            if len(line) > 2 and line.find(':'):
                output += '  %s\n' % (line if len(line) <=120 else '{0} (truncated)'.format(line[:120]))


    with tempfile.NamedTemporaryFile(mode='wb',prefix='h.', delete=False) as header_file:
        header_file.write(first_line)
        header_file.write(header)
        header_file.close()
        command_string += '--header={0} '.format(header_file.name)
    
    with tempfile.NamedTemporaryFile(mode='wb',prefix='c.', delete=False) as content_file:
        if direction == 0:
            content_file.write(data)
            content_file.close()
            command_string += content_file.name
        elif data:
            content_file.write(data)
            content_file.close()
            command_string += content_file.name
        else:
            content_file.close()
            os.unlink(content_file.name)
            command_string += samples[random.randint(0,len(samples)-1)]

    command_list=command_string.split(' ')
    output += '  %s\n' % command_string
    p = subprocess.Popen(['ls', '-l', 'http.pcap'], stdout=subprocess.PIPE)
    outstr, err = p.communicate()
    output += '  %s\n' % outstr
    print output
    #os.unlink(header_file.name)

def split_raw_httpmessage(s):
    crlf='\r\n'
    crlfcrlf = '\r\n\r\n'
    end_idx=str(s).find(crlfcrlf)
    if end_idx <= 0:
        return None
    idx=str(s).find(crlf)
    first_line = s[:idx]
    headers=s[idx:end_idx+4]
    data=s[end_idx+4:]
    return first_line,headers,data

def decode_http_packet(s):
    direction=1
    
    if re.search('^GET', s):
        pass
    elif re.search('^POST', s):
        pass
    elif re.search('^HTTP', s):
        direction=0
    else:
        return False
    args=split_raw_httpmessage(s)
    if args:
        request_line,headers,data=args
        scan_by_cca(direction,request_line,headers,data)
    return True

class Unpacking(object):
    daemon_threads = False
    def __init__(self, pktlen, data, timestamp):
        self.pktlen = pktlen
        self.data = data
        self.timestamp = timestamp
        self.unpack()

    def process_ip_packet(self, pktlen, data, timestamp):
        if not data:
            return
        
        if data[12:14]=='\x08\x00':
            decoded=decode_ip_packet(data[14:])
        else:
            return 
        if protocols[decoded['protocol']] == 'tcp':
            tcp=decode_tcp_packet(decoded['data'])
            logging.debug('%s.%f %s:%d > %s:%d %s %d' % (time.strftime('%H:%M', time.localtime(timestamp)), timestamp % 60,
                                             decoded['srcaddr'], tcp['srcport'],
                                             decoded['dstaddr'], tcp['dstport'],
                                             decode_tcp_flags(tcp['flags']),
                                             len(tcp['data'])))
            
            if not filter_tcp_flags(tcp['flags']):
                decode_http_packet(tcp['data'])
            
        
    def unpack(self):
        t = threading.Thread(target = self.process_ip_packet,
                                 args = (self.pktlen, self.data, self.timestamp))
        t.daemon = False
        t.start()

if __name__=='__main__':

    p = pcap.pcapObject()
    #dev = pcap.lookupdev()
    #dev = sys.argv[1]
    #net, mask = pcap.lookupnet(dev)
    # note:    to_ms does nothing on linux
    #p.open_live(dev, 1600, 0, 100)
    p.open_offline('http.pcap')
    #p.dump_open(
    p.setfilter(string.join(sys.argv[1:],' '), 0, 0)

    # try-except block to catch keyboard interrupt.    Failure to shut
    # down cleanly can result in the interface not being taken out of promisc.
    # mode
    #p.setnonblock(1)
    try:
        while 1:
            #p.dispatch(1, print_packet)
            args=p.next()
            if args:
                while (threading.active_count() > 4):
                    time.sleep(0.01)
                    continue
                unpack=Unpacking(*args)
                #print_packet(*args)
            else:
                break
            
    except KeyboardInterrupt:
        print '%s' % sys.exc_type
        print 'shutting down'
        print '%d packets received, %d packets dropped, %d packets dropped by interface' % p.stats()
    
