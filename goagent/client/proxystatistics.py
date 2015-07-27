#!/usr/bin/env python
# coding:utf-8

import os
import sys
import sysconfig
import sqlite3
import urllib2
import socket
import tldextract
import time
import getopt
import tempfile
import logging
import re
import cStringIO
from urlparse import urlparse

sys.path += [os.path.abspath(os.path.join(__file__, '../packages.egg/%s' % x)) for x in ('noarch', sysconfig.get_platform().split('-')[0])]
show_fullurl = False

def NormalizeDBName(dbname):
    normailze_db_name = ''
    if os.name == 'nt':
        normailze_db_name = tempfile.gettempdir()+'\\sqlitedb\\'+dbname
    else:
        normailze_db_name = tempfile.gettempdir()+'/sqlitedb/'+dbname
    normailze_path = os.path.dirname(normailze_db_name)
    if not os.path.exists(normailze_path): os.makedirs(normailze_path)
    return normailze_db_name

def NormalizeURL(originurl, ulen=80, full = False):
    if full:
        return originurl
    surl = originurl
    urllen = len(originurl)
    if urllen > ulen:
        noquery = originurl.split('?')[0]
        if len(noquery) > ulen:
            o = urlparse(noquery)
            surl = o.scheme+'://'+o.netloc+'/...'
        else:
            surl = noquery+'?...'
    return surl

def TestURLConnectivity(url):
    resp = {}
    urlstr = NormalizeURL(url, full=show_fullurl)    
    try:
        t0 = time.time()
        f = urllib2.urlopen(url, timeout=3)
        resp['time'] = time.time()-t0
        resp['status'] = f.getcode()
        contnet_len = int(f.info().get('Content-Length') or '0')
        if (contnet_len > 0):
            f.read(contnet_len)        
        if f.getcode() == 200:
            print '%-100s - ok %d %f' % (urlstr,contnet_len, resp['time'])
            return resp
        else:
            print '%-100s - status code : %d' % (urlstr, f.getcode())
    except (urllib2.URLError, urllib2.HTTPError, socket.error, OSError) as err:   
        print '%-100s - %s' % (urlstr, err)    
    
    return None

def ExecuteSQL(conn, sql):
    cursor = conn.cursor()
    try:
        cursor.execute(sql)
        conn.commit()
    except sqlite3.OperationalError as err:
        logging.error('execute(%s) - %s', sql, err)
        conn.commit()    

def host_is_valid(host):
    if not host:
        return False
    try:
        ext = tldextract.extract('http://%s' %  host)
        if ext.domain and ext.suffix:
            return True
        return False
    except:
        return False

class WritelistDB(object):
    db_name = 'whitelist.db.%s' % (socket.gethostname().split('.')[0].lower())
    db_name = NormalizeDBName(db_name)
    tb_name = 'Domains'
    wl_file = 'whitelist.txt'
    pacfile = 'autoconfig.js'

    def __init__(self):
        self.conn = sqlite3.connect(self.db_name)
        c = self.conn.cursor()
# Create table if not exists
        sql = '''CREATE TABLE IF Not Exists %s (
                     name Text UNIQUE NOT NULL,                     
                     country_code TEXT,
                     manual INTEGER default 0,
                     automatic INTEGER default 0,
                     speed REAL default 0.0)''' % self.tb_name
        c.execute(sql)
        self.conn.commit()
        self.default()
    
    def __del__(self):
        self.conn.close()
    
    def update(self, domain_list, autox=1):
        c = self.conn.cursor()
        for domain in domain_list:
            if not host_is_valid(domain['name']):
                continue
            sql = 'SELECT manual from %s WHERE name = "%s"' % (self.tb_name, domain['name'])
            c.execute(sql)
            row = c.fetchone()
            manual = int(row[0]) if (row and row[0]) else 0
            if (manual):
                continue
            m = 1 if autox == 0 else 0
            sql = "INSERT OR REPLACE INTO %s(Name,country_code,automatic,manual,speed) VALUES ('%s', '%s', %d, %d,'%s')" % (
                    self.tb_name, domain['name'], domain['code'], autox, m, domain['time'])
            c.execute(sql)
        self.conn.commit()
    
    def default(self, clearall = False, names = []):
        c = self.conn.cursor()
        if clearall:
            c.execute('DELETE FROM %s' % (self.tb_name))
        
        # add default domains to whitelist DB
        with open(self.wl_file) as fp:
            lines = [line for line in fp if line.strip()]
            fp.close()
            for n in names: lines.append(n)
            lines.sort()
            with open(self.wl_file, 'w') as fp:
                i=0
                for line in lines:
                    domain = line.strip()
                    i+=1
                    if len(domain) == 0:
                        continue
                    sql = "INSERT OR REPLACE INTO %s(Name,manual) VALUES ('%s', 1)" % (self.tb_name, domain)
                    c.execute(sql)                    
                    fp.write('%s\n' % domain) if (i < len(lines)) else fp.write('%s' % domain)                             
                fp.close()
        self.conn.commit()
                        
    def delete(self, domain):
        c = self.conn.cursor()
        sql = "DELETE from %s where name='%s'" % (self.tb_name, domain)
        c.execute(sql)
        self.conn.commit()

    def dump(self, domains = [], change_pac = False):
        def update_whitelist(varwl, pacfile):
            f = tempfile.NamedTemporaryFile(mode='w', prefix='whitelist_', suffix='.txt', delete=False)
            f.write(varwl)
            f.close()
            print 'New whitelist file - ', f.name
            
            m = re.compile('.*var whitelist_hosts.*', flags=0)
            with open(pacfile, 'r') as infile:
                output = cStringIO.StringIO()
                for line in infile:
                    if m.match(line):
                        output.write(varwl)
                    else:
                        output.write(line)                
                infile.close()
                with open(pacfile, 'w') as infile:
                    content = output.getvalue()
                    print '%d bytes write to file %s' % (len(content), pacfile)
                    infile.write(content)
                    infile.close()
                    output.close()

        sql = "SELECT * FROM %s ORDER BY manual DESC" % self.tb_name
        if len(domains) > 0:
            domains = str(domains).lstrip('[')
            domains = str(domains).rstrip(']')
            sql = "SELECT * FROM %s WHERE Name IN (%s) ORDER BY manual" % (self.tb_name, domains)
        c = self.conn.cursor()
        c.execute(sql)
        
        rows = c.fetchall()
        max_len = 0
        for row in rows:
            max_len = len(row[0]) if max_len < len(row[0]) else max_len
        
        rowcount=0
        varwl = '\tvar whitelist_hosts = {'
        sformat = '{:<%d} {:<7} {:<4} {:<5} {:.5}' % (max_len+4)
        print sformat.format('Domain', 'Code', 'M', 'A', 'Speed')
        for row in rows:
            print sformat.format(row[0],row[1],row[2],row[3],row[4])
            if (len(domains) > 0):
                StatDB.dump(row[0])
            varwl += '"%s":%.3f,'%(row[0].encode('ascii','ignore'), row[4])
            rowcount+=1
        print '{0}'.format(rowcount)
        varwl = varwl.rstrip(',')+'};\r\n'
        if change_pac: update_whitelist(varwl, self.pacfile)

class GoogleDB(object):
    db_name = NormalizeDBName('google.db')
    tb_name = 'Google'
    tb_test = 'IpTest'
    def __init__(self,dbname='', copy_default=False):
        if dbname:
            dbname = NormalizeDBName(dbname)
            if copy_default:
                import shutil
                shutil.copyfile(self.db_name, dbname)
            self.db_name = dbname
        self.conn = sqlite3.connect(self.db_name)        
# Create table if not exists
        sql = '''CREATE TABLE IF Not Exists %s (
                     host Text NOT NULL,
                     addr Text NOT NULL,
                     region TEXT default "",
                     time REAL default 0.0,
                     handshake REAL default 0.0,
                     cert BLOB,
                     flags INTEGER default 0)''' % self.tb_name
        
        ExecuteSQL(self.conn, sql)
        sql = '''CREATE TABLE IF Not Exists %s (                     
                     addr Text NOT NULL,
                     region TEXT default "",
                     connect REAL default 0.0,
                     handshake REAL default 0.0,
                     commonname TEXT,
                     flags INTEGER default 0)''' % self.tb_test
            
        ExecuteSQL(self.conn, sql)
        self.ip_cache = []
    
    def execute_many(self, sql, *args):
        try:
            self.conn.executemany(sql, self.ip_cache)
            self.conn.commit()
            return True
        except sqlite3.OperationalError as err:
            logging.error('executemany(%s) - %s', sql, err)
            self.conn.commit()
            return False

    def insert(self, **kwargs):
        host = kwargs.get('host', None)
        if not host or not host_is_valid(host):
            return
        addr = kwargs.get('ip', None)
        if not addr:
            return
        handshake = kwargs.get('handshake', None)
        if not handshake:
            return
        
        self.ip_cache.append((host,addr,time.time(),handshake))
        if self.ip_cache > 20:
            sql = "INSERT INTO %s(host,addr,time,handshake) VALUES (?, ?, ?, ?)" % (self.tb_name)
            if self.execute_many(sql, self.ip_cache):
                del self.ip_cache[:]
                
    def update(self,**kwargs):
        addr = kwargs.get('ip', None)
        if not addr:
            return
        region = kwargs.get('region', None)
        if not region:
            return
        sql = "UPDATE %s SET region='%s' where addr='%s'" % (self.tb_name,region,addr)
        ExecuteSQL(self.conn, sql)
    
    def dump(self):
        import pygeoip
        geoip = pygeoip.GeoIP('GeoIP.dat')
        sql = "SELECT * FROM %s order by handshake" % self.tb_name
        c = self.conn.cursor()
        c.execute(sql)
        self.conn.commit()
        rows = c.fetchall()
        max_len = 0
        for row in rows:
            max_len = len(row[0]) if max_len < len(row[0]) else max_len
            code=geoip.country_code_by_addr(row[1])
            self.update(ip=row[1],region=code)
            print 'update %s %s' % (row[1],code)
        sformat = '{:<%d} {:<20} {:<4} {:<6} {:<5}' % (max_len+2)
        for row in c.execute(sql):
            print sformat.format(row[0],row[1],row[2],row[4],row[6])

    def delete_ip(self):
        sql = "DELETE from %s" % (self.tb_test)
        ExecuteSQL(self.conn, sql)

    def insert_ip(self, addr, region, connect=0.0, handshake=0.0, cn='', flags=0):
        self.ip_cache.append((addr, region, connect, handshake, cn, flags))
        if self.ip_cache > 10:
            sql = "INSERT INTO %s VALUES (?, ?, ?, ?, ?, ?)" % (self.tb_test)
            if self.execute_many(sql, self.ip_cache):
                del self.ip_cache[:]
    
    def dump_ip(self):
        sql = "INSERT INTO %s VALUES (?, ?, ?, ?, ?, ?) limit 100" % (self.tb_test)
        if len(self.ip_cache) > 0:
            self.execute_many(sql, self.ip_cache)
            del self.ip_cache[:]
        sql = "SELECT * FROM %s order by handshake" % (self.tb_test)
        c = self.conn.cursor()
        print '------------------------------'
        ipaddr=''
        for row in c.execute(sql):
            print '%-20s %-4s %7.3f %9.6f %-40s %d' % (row[0],row[1],row[2],row[3],row[4],row[5])
            ipaddr+='{0}|'.format(row[0]) if str(row[4]).endswith('googleapis.com') else ''
        f = tempfile.NamedTemporaryFile(mode='w', prefix='googleip_', suffix='.txt', delete=False)
        f.write(ipaddr.rstrip('|'))
        f.close()
        print 'Google IP write to %s' % f.name

class StatisticsDB(object):
    db_name = NormalizeDBName('region.db')    
    tb_name = 'Hosts'
    flags_connected = 0x01
    flags_badstatus = 0x02
    flags_blocked = 0x80
    def __init__(self):        
        self.conn = sqlite3.connect(self.db_name)        
# Create table if not exists
        sql = '''CREATE TABLE IF Not Exists %s (
                     name Text UNIQUE NOT NULL,
                     time TEXT default CURRENT_TIMESTAMP,
                     update_time INTEGER default 0,
                     country_code TEXT,                     
                     originurl TEXT,
                     gaecount INTEGER default 0,
                     gaespeed REAL default 0.0,
                     directcount INTEGER default 0,                                          
                     flags INTEGER default 0)''' % self.tb_name
        ExecuteSQL(self.conn, sql)

    def __del__(self):
        self.conn.close()

    def insert(self, **kwargs):        
        host = kwargs.get('host', None)
        if not host or not host_is_valid(host):
            return        
        code = kwargs.get('code', 'unknown')
        sql = "SELECT country_code from %s where name='%s'" % (self.tb_name, host)
        c = self.conn.cursor()        
        c.execute(sql)
        r = c.fetchone()
        if not r:
            sql = "INSERT INTO %s(Name,country_code,update_time) VALUES ('%s', '%s', %d)" % (
                              self.tb_name, host, code, int(time.time()))
            ExecuteSQL(self.conn, sql)
        elif r[0]:
            old_code = r[0]
            if (old_code != 'code'):
                sql = "UPDATE %s SET country_code='%s',update_time=%d WHERE name='%s'" % (
                              self.tb_name, code, int(time.time()), host)
                ExecuteSQL(self.conn, sql)
        else:            
            pass

    def delete(self, domains=[], deleteall = False):
        if deleteall:
            sql = "DELETE from %s" % (self.tb_name)
            ExecuteSQL(self.conn, sql)
        else:
            for host in domains:
                sql = "DELETE from %s where name LIKE '%s%s'" % (self.tb_name, '%', host)
                ExecuteSQL(self.conn, sql)
        
    def reset(self, **kwargs):
        column = kwargs.get('column', '')
        if column == 'flags':
            sql = "UPDATE %s set flags=0" % (self.tb_name)
            ExecuteSQL(self.conn, sql)
        if column == 'gaecount':
            sql = "UPDATE %s set gaecount=0" % (self.tb_name)
            ExecuteSQL(self.conn, sql)
        if column == 'directcount':
            sql = "UPDATE %s set directcount=0" % (self.tb_name)
            ExecuteSQL(self.conn, sql)

    def update_flags(self, host, flags):
        c = self.conn.cursor()
        sql = "SELECT flags from %s where name='%s'" % (self.tb_name, host)
        c.execute(sql)
        r = c.fetchone()
        if r:
            sql = "Update %s Set flags=%d,update_time=%d where name='%s'" % (self.tb_name,flags,int(time.time()),host)
            ExecuteSQL(self.conn, sql)

    def update_gae_count(self, host, method='', url='', speed=0.0):
        if not host_is_valid(host):
            return
        c = self.conn.cursor()
        sql = "SELECT gaecount,gaespeed from %s where name='%s'" % (self.tb_name, host)
        c.execute(sql)
        r = c.fetchone()
        if r:
            count=int(r[0])+1
            speed+=float(r[1])
            sql = "UPDATE %s SET gaecount=%d,gaespeed=%.3f,update_time=%d%s where name='%s'" % (
                   self.tb_name, count, speed, int(time.time()), 
                   (",originurl='%s'" % url) if method == 'GET' else '', host)
            #print '****',sql
            #logging.info(sql)
            ExecuteSQL(self.conn, sql)

    def update_direct_count(self, host, method='', url='', speed=0.0):
        if not host_is_valid(host):
            return
        c = self.conn.cursor()
        sql = "SELECT directcount from %s where name='%s'" % (self.tb_name, host)
        c.execute(sql)
        r = c.fetchone()
        if r:
            count=int(r[0])+1
            sql = "Update %s Set directcount=%d,update_time=%d%s where name='%s'" % (
                self.tb_name, count, int(time.time()),
                (",originurl='%s'" % url) if method == 'GET' else '', host)
            ExecuteSQL(self.conn, sql)

    def dump(self, domain = None, wl=None):
        with_domain = False
        sql = "SELECT * FROM %s ORDER BY gaecount DESC" % self.tb_name
        if domain:
            with_domain = True
            sql = 'SELECT * FROM %s WHERE name LIKE \'%s%s\' ORDER BY gaecount DESC' % (self.tb_name, '%',domain)
            
        c = self.conn.cursor()
        c.execute(sql)
        self.conn.commit()
        rows = c.fetchall()
        max_len = 0
        for row in rows:
            max_len = len(row[0]) if max_len < len(row[0]) else max_len
        sformat = '{:<%d} {:<7} {:<4}{:<6} {:<5} {:<5} {}' % (max_len+4)
        if not with_domain:
            print sformat.format('Host', 'Code', 'GAE/', 'Direct', 'flags', 'speed', 'URL')
        for row in rows:
            url = NormalizeURL(row[4], full=show_fullurl) if row[4] else ''
            final_fmt = sformat
            if with_domain:
                final_fmt = '  '+sformat
            print final_fmt.format(row[0],row[3],row[5],row[7], row[8], row[6], url)
        self.update_whitelist(rows, whitelist=wl)

    def update_whitelist(self, rows, **kwargs):
        wl = kwargs.get('whitelist', None)
        domains_wl = []
        curr_time=int(time.time())

        for row in rows:
            update_time=int(row[2])
            gae_count=int(row[5])            
            dir_count=int(row[7])
            old_flags=int(row[8])            
            if (dir_count==0 and gae_count==0) or not row[4] or (old_flags !=0 and update_time+7200>curr_time): # for url
                continue
            gae_total_time=float(row[6])  
            gae_average_time=gae_total_time/(gae_count) if gae_count > 0 else 0.0
            host = row[0]
            url = row[4].encode('ascii','ignore')         
            ext = tldextract.extract('http://%s' %  host)            
            ret = TestURLConnectivity(url)
            if ret:
                if ret['status'] == 200:
                    domain_name = '%s.%s' % (ext.domain, ext.suffix)                  
                    domain = {'name':domain_name, 'code':row[3], 'time':ret['time']}
                    if (ret['time'] < 0.5 or ret['time'] < gae_average_time):
                        domains_wl.append(domain)
                    self.update_flags(host, self.flags_connected)
                else:                    
                    self.update_flags(host, self.flags_badstatus)
            else:
                self.update_flags(host, self.flags_blocked)
                domain_name = '%s.%s' % (ext.domain, ext.suffix)
                wl.delete(domain_name)
        # Test connectivity for Direct hosts
        wl.update(domains_wl)

StatDB = StatisticsDB()
GoogleStatus = GoogleDB()

global_options='''
OPTIONS
-h       show this manual
-a       add whitelists
--dump   dump both databases, test connectivity and update whitelist databases
         if -f option specified, show full URL
         if -i option specified, then update PAC file immediately
         if specify domains only dump specific domains
-W/-S/-G database indicator, only operate on Whitelist/Statistics/Google database
-d       if domains specified, only delete specific domain from database, else delete all
--reset  reset specific column(flags|gaecount|directcount) specified by --reset-flags= option
         for whitelist reset database from default whitelist.txt
'''
def usage(prog='proxystatistics.py'):
    print 'Usage: %s [options] [domains]' % prog
    print global_options
    sys.exit(0)

def main():
    opt_dump = False
    opt_g_db = False
    opt_g_copy = False
    opt_wl_db = False
    opt_st_db = False
    opt_add = False
    opt_del = False
    opt_reset = False
    opt_reset_flag = -1
    opt_inreplace = False
    opt_domains = []
    
    wl = WritelistDB()
    opts, args = getopt.getopt(sys.argv[1:], "adfhiWSG:", ['reset','reset-flags=','dump'])
    for o, a in opts:
        if o == "-h":
            usage()
        elif o == '-f':
            show_fullurl = True
        elif o == "-a":
            opt_add = True
        elif o == "-d":
            opt_del = True
        elif o == "-i":
            opt_inreplace = True
        elif o == "-W":
            opt_wl_db = True
        elif o == "-S":
            opt_st_db = True
        elif o == "-G":
            opt_g_db = True
            if int(a) > 0:
                opt_g_copy = True
        elif o == "--reset":
            opt_reset = True
        elif o == "--reset-flags":
            opt_reset_flag = a
        elif o == "--dump":
            opt_dump = True             
        else:
            usage()

    if len(args) > 0:
        opt_domains = args[0:]
    
    if opt_add:
        print 'Add new whitelists'
        wl.default(True, opt_domains)
        wl.dump(change_pac=True)
        return 

    if opt_del:
        wl.delete(opt_domains) if opt_wl_db else ''
        StatDB.delete(opt_domains, True if len(opt_domains)==0 else False) if opt_st_db else ''
    
    if opt_reset:
        StatDB.reset(column=opt_reset_flag) if opt_st_db else ''
        wl.default(True) if opt_wl_db else ''

    if opt_dump:
        if opt_g_db:
            G = GoogleDB('google_copy.db', copy_default=opt_g_copy)
            G.dump()
            G.dump_ip()
        else:
            StatDB.dump(opt_domains, wl)
            wl.dump(change_pac=opt_inreplace)

if __name__ == '__main__':
    main()

