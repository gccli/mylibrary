#!/usr/bin/env python
# coding:utf-8

import sqlite3

class DB(object):
    db_name = 'region.db'
    tb_name = 'Hosts'
    
    def __init__(self):
        self.conn = sqlite3.connect(self.db_name)
        c = self.conn.cursor()
# Create table if not exists
        sql = 'CREATE TABLE IF Not Exists %s (Name Text UNIQUE NOT NULL, time TEXT default CURRENT_TIMESTAMP, country_code TEXT, addresses TEXT, flags INTEGER)' % self.tb_name
        c.execute(sql)
        self.conn.commit()

    def __del__(self):
        self.conn.close()

    def insert(self, host, code, iplist):
        c = self.conn.cursor()
        addresses=''
        for ip in iplist:
            addresses += '%s,' % ip
        addresses=addresses.rstrip(',')
        sql = "INSERT OR REPLACE INTO Hosts(Name,country_code,addresses) VALUES ('%s', '%s', '%s')" % (host, code if code else 'unknown', addresses)
        print sql
        c.execute(sql)
        self.conn.commit()
        
    def update_flags(self, host, flags):
        c = self.conn.cursor()
        sql = "Update Hosts Set flags=%d where host='%s'" % (flags,host)
        c.execute(sql)
        self.conn.commit()

    def query_iplist(self, host):
        c = self.conn.cursor()
        c.execute("SELECT addresses FROM Hosts WHERE Name = '%s'" % host)
        r = c.fetchone()
        return r[0].split(',')

if __name__ == '__main__':
    db = DB()
    db.insert('www.baidu.com', 'cn', ['61.135.169.105'])
    db.query_iplist('www.baidu.com')

    db.insert('www.baidu.com', 'cn', ['61.135.169.105', '61.135.169.125'])
    iplist = db.query_iplist('www.baidu.com')
    print iplist

    db.insert('www.baidu.com', 'cn', [])
    db.query_iplist('www.baidu.com')

    del db
