#! /usr/bin/python 

import re
import sys
import time
from datetime import datetime
import urllib2
import socket
import getopt
from io import StringIO
import lxml
import lxml.html

import numpy
import pandas as pd
from pandas import Series, DataFrame
import pandas.io.data as web

from collections import OrderedDict as odict

currencies = odict({
    'USD':'United States Dollar',
    'CNY':'Chinese Yuan',
    'CAD':'Canadian Dollar',
    'GBP':'GreatBritain Pound',
    'AUD':'Australian Dollar',
    'RUB':'Russian Rouble',
    'EUR':'Euro',
    'CHF':'Swiss francs',
    'JPY':'Japanese Yen',
    'KRW':'South Korean Won'
})

# http://www.jarloo.com/yahoo_finance/
# http://www.icbc.com.cn/ICBCDynamicSite/Charts/GoldTendencyPicture.aspx
opt_verbose = 0
opt_timeout = 20.0
opt_average = 235.71
opt_number = 60.0
opt_content_length = 1024
opt_noout = True
ounce = 31.103481

base = 'http://download.finance.yahoo.com/d/quotes.csv?'
query = 's=usdcny=x+xauusd=x+xaucny=x&f=sd1t1npoab'
#USDEUR=X+USDAUD=X+USDJPY=X+USDGBP=X+USDCNY=X
#http://download.finance.yahoo.com/d/quotes.csv?s=USDEUR=X+USDAUD=X+USDJPY=X+USDGBP=X+USDCNY=X&f=sd1t1npoab

usdcny = 0.0

def exchange_rate():
    global usdcny
    query = 's='
    for i in currencies.keys():
        if i == 'USD': continue
        query += 'USD{0}=X+'.format(i)
    fmt = '&f=nab'
    url = 'http://finance.yahoo.com/q?'+query.rstrip('+')
    print url.lower()

    query = query.rstrip('+') + fmt
    url = base + query
    f = urllib2.urlopen(url, timeout=opt_timeout)
    if f.getcode() != 200:
        print f.info()
        sys.exit(0)
    content = f.read()
    if content:
        data = pd.read_csv(StringIO(unicode(content)),  names = ['Name', 'Ask', 'Bid'], header=None)
        print data.to_string(index=False)
        usdcny = float(data[data.Name == 'USD/CNY'].Bid)

def main():
    global opt_verbose
    global opt_timeout
    global opt_number
    global opt_average
    opt_loop_time = 5
    opt_loop = False
    try:
        opts,args = getopt.getopt(sys.argv[1:], "va:n:", ["timeout=", "loop"])
    except getopt.GetoptError as err:
        print str(err)
        sys.exit(2)

    for o, a in opts:
        if o in ("-v"):
            opt_verbose = 1
        elif o in ("-a"):
            opt_average = float(a)
        elif o in ("-n"):
            opt_number = float(a)
        elif o in ("--loop"):
            opt_loop = True
        elif o in ("--timeout"):
            opt_timeout = float(a)
        else:
            sys.exit(2)

    exchange_rate()
    print usdcny
    for i in xrange(1160, 1240+1):
        print '| %d - %.3f' % (i, i*usdcny/ounce),' | ',
        if ((i+1)%5 == 0):
            print ''
    sys.exit(0)

if __name__ == '__main__':
    main()

