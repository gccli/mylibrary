#! /usr/bin/python 

import re
import sys
import time
from datetime import datetime
import urllib2
import socket
import getopt

import lxml
import lxml.html
import threading
import thread
import Queue
import urllib
import tempfile
import pytz
import numpy
import pandas as pd
from pandas import Series, DataFrame
import pandas.io.data as web

from collections import OrderedDict as odict
from io import StringIO

class MyStock(object):
    timeout = 10
    debug = False

    stocks = {'600740.SS':{'average':8, 'number':600},
              '600036.SS':{'average':15.2, 'number':1800},
              }

    profit = Series(0., index=xrange(0, len(stocks.keys())))
    header = ['Symbol', 'Ask', 'Bid', 'Time', 'CHG & Percent', 'Open', 'Close', 'LOW', 'HIGH', 'VOLUME']
    hdrfmt = 'sb2b3t1copghv'

    def __init__(self):
        base_format = 'http://download.finance.yahoo.com/d/quotes.csv?s={0}&f={1}&e=.csv'
        self.url = base_format.format('+'.join(self.stocks.keys()), self.hdrfmt)
        
        total = {'total':0.0}
        for k,v in self.stocks.items():
            total[k]=float(v['average'])*float(v['number'])
            total['total'] += total[k]
        for k,v in self.stocks.items():
            percent = total[k]/total['total']
            print k, '%-10.1f %.2f%%'%(total[k], percent*100)
        self.total = total['total']

    def query(self):
        f = urllib2.urlopen(self.url, timeout=self.timeout)
        if f.getcode() != 200:
            print f.info()
            sys.exit(0)
        # http://pandas.pydata.org/pandas-docs/stable/io.html
        content = f.read()
        data = pd.read_csv(StringIO(unicode(content)), names=self.header, parse_dates=True, header=None)
        stocks  = self.stocks
        pd.set_option('display.width', 120)

        i=0; total = 0.0
        for k,v in self.stocks.items():
            s =  data[data.Symbol == k]

            if v['number'] == 0.0:
                self.profit[i] = 0
                i+=1
                continue
            val = (s.Bid-v['average'])*v['number']
            self.profit[i] = float(val)
            total += self.profit[i]

            i+=1
        data['Profit'] = self.profit
        return data,total,self.total

    def parse_sina_html(self, url, queue):
        parser = lxml.etree.HTMLParser(encoding='utf-8')
        #parser = lxml.etree.HTMLParser()
        f = urllib.urlopen(url)
        if f.getcode() == 200:
            fp = tempfile.NamedTemporaryFile(mode='w', prefix='stock_', suffix='.html', delete=False)
            fp.write(f.read())

            root = lxml.html.parse(fp.name, parser).getroot()
            div = root.get_element_by_id('hq')
            change = div.get_element_by_id('change')
            changeP = div.get_element_by_id('changeP')
            price =  div.get_element_by_id('price')
            print change,changeP,price
            fp.close()
        
    def sina(self):
        url_format = 'http://finance.sina.com.cn/realstock/company/{0}{1}/nc.shtml'
        urls = []
        for symbol in self.stocks.keys():
            n,s = tuple(symbol.split('.'))
            urls.append(url_format.format('sh' if s == 'SS' else 'sz', n))
        result_queue=Queue.Queue()
        for url in urls:
            thread.start_new_thread(self.parse_sina_html, (url, result_queue))

        while True:
            try:
                r = result_queue.get(timeout=3)
            except Queue.Empty as err:
                print 'Done'
            break

def main():
    beijing = pytz.timezone('Asia/Shanghai')
    eastern = pytz.timezone('US/Eastern')
    stock = MyStock()

    while True:
        data,total,capital=stock.query()
        t = 'N/A'
        if data.Time[0] is not numpy.nan:
            try:
                t = datetime.strptime(data.Time[0],"%H:%M%p")
                t_eastern = eastern.localize(t,is_dst=None)
                t_beijing = t_eastern.astimezone(beijing)
                t = t_beijing.strftime("%H:%M")
            except:
                print 'datetime is', data.Time
        time.sleep(2.0)
        print "\033[2J\033[1;1H"
        print '\033[ATotal : %.2f (%.2f%% of %.2f)   time:%s' % (total,100.0*total/capital,capital,t)
        print data.to_string(index=False)

if __name__ == '__main__':
    main()
