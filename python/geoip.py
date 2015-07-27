#! /usr/bin/env python

import os
import sys
import getopt
import time
import socket
import pygeoip
import urllib2
import gzip
import geoip2
import geoip2.database
import geoip2.errors
import maxminddb

# reference
# https://github.com/maxmind/GeoIP2-python

def gunzip(file_name):
    out_file=file_name.strip('.gz')
    print 'Decompressing "%s" to "%s" ...' % (file_name,out_file)

    f = gzip.open(file_name, 'rb')
    file_content = f.read()
    f.close()
    with open(out_file, 'wb') as outf:
        outf.write(file_content)
        outf.close()


def download_unzip(url):
    file_name = url.split('/')[-1]
    u = urllib2.urlopen(url)
    f = open(file_name, 'wb')
    meta = u.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "Downloading: %s Bytes: %s" % (file_name, file_size)

    file_size_dl = 0
    block_sz = 8192
    while True:
        buffer = u.read(block_sz)
        if not buffer:
            break

        file_size_dl += len(buffer)
        f.write(buffer)
        status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
        status = status + chr(8)*(len(status)+1)
        print status,

    f.close()
    gunzip(file_name)
    os.unlink(file_name)

def usage():
    print 'geoip.py [ -a ] [ -d db ] target'
    sys.exit(0)

def resolv_host(host):
    iplist = []
    start=time.time()
    print '\033[32mResolving host "%s" ...\033[0m' % host
    try:
        iplist = socket.gethostbyname_ex(host)[-1]
        diff=time.time()-start
        print '\033[A\033[2K\033[32m  %s - %s (%lf)\033[0m' % (host,iplist,diff)
    except socket.error as e:
        print '\033[A\033[2K'

    return iplist

def GeoIP_Legacy(hosts, db_file='GeoIP.dat'):
    if not os.path.exists(db_file):
        download_unzip('http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz')
        db_file = 'GeoIP.dat'
    try:
        print '******** Try to load database "%s" ...' % db_file
        gi = pygeoip.GeoIP(db_file)
    except:
        print 'failed to open %s' % db_file
        sys.exit(1)

    print '  %-16s Code  CountryName' % 'Host'
    for host in hosts:
        iplist = resolv_host(host)
        for ip in iplist:
            start=time.time()
            code=gi.country_code_by_addr(ip)
            name=gi.country_name_by_addr(ip)
            diff=time.time()-start
            print '  %-16s %-4s  %s (%lf)' % (ip, code, name, diff)

def GeoIP2(hosts, db_file='GeoLite2-City.mmdb'):
    if not os.path.exists(db_file):
        download_unzip('http://geolite.maxmind.com/download/geoip/database/GeoLite2-City.mmdb.gz')
        db_file = 'GeoLite2-City.mmdb'
    try:
        print '******** Try to load database "%s" ...' % db_file
        reader = geoip2.database.Reader(db_file)
    except maxminddb.InvalidDatabaseError as e:
        print 'failed to open %s' % db_file
        print e
        sys.exit(1)

    print '  %-16s %-16s Code %-32s %-16s Location'%('Address', 'Country', 'Province', 'City')
    for host in hosts:
        iplist = resolv_host(host)
        for ip in iplist:
            try:
                c = reader.city(ip)
                print '  %-16s %-16s %-4s %-32s %-16s (%lf %lf)' % (
                    ip,c.country.name,c.country.iso_code,c.subdivisions.most_specific.name,c.city.name,
                    c.location.latitude, c.location.longitude)
            except geoip2.errors.AddressNotFoundError as e:
                print '\033[A\033[2K\033[33m  %s - address not found\033[0m' % ip
            except:
                pass

    reader.close()

def main():
    # hosts for test
    known_hosts = []
    #known_hosts = ['www.google.com',
    #               'baidu.com',
    #               'taobao.com',
    #               'alipay.com',
    #               'qq.com',
    #               '163.com',
    #               'sina.com.cn',
    #               'www.facebook.com',
    #               'amazon.com',
    #               'python.org',
    #'www.ylnet.com.cn',
    #               'www.bjld.gov.cn',
    #               'www.12306.cn',
    #               'www.nssh.gov.cn'
    #               ]

    opts, args = getopt.getopt(sys.argv[1:], "d:")
    for o, a in opts:
        if o == "-d":
            db_file = a
        else:
            usage()
    if not args:
        usage()
    target = args[0:]
    hosts = target+known_hosts

    GeoIP2(hosts)
    print ''
    GeoIP_Legacy(hosts)

main()
