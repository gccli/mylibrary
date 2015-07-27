#!/usr/bin/ipython

import json
import time
import lxml
import lxml.html

from datetime import datetime as dt
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.dates import YearLocator, MonthLocator, DateFormatter, HourLocator, MinuteLocator, SecondLocator
from matplotlib.lines import Line2D

opt_noout = False

class ICBC(object):
    xdata = []
    ydata = []
    average = 0.0
    number = 0.0
    alarm = 236.0
    last = {
        'CNY':{'ask':0.0, 'bid':0.0, 'mid':0.0, 'high':0.0, 'low':0.0},
        'USD':{'ask':0.0, 'bid':0.0, 'mid':0.0, 'high':0.0, 'low':0.0},
        'time':0
    }

    def __init__(self, bidprice, bidnumber, daily_file = 'icbc_daily.csv'):
        self.average = bidprice
        self.number = bidnumber
        self.daily_data = pd.read_csv(daily_file, parse_dates = True, index_col=0)

    def current(self):
        curr = int(time.time())%86400
        return curr

    def draw_daily(self, ax1):
        self.daily_data.Close.plot(ax = ax1, style='g', label='close')
        self.daily_data.Open.plot(ax = ax1, color='k', linestyle='dashed', label='open')
        lowest = self.daily_data.Low.min()
        highest = self.daily_data.High.max()
        x = self.daily_data[self.daily_data.Low == lowest]
        y = self.daily_data[self.daily_data.High == highest]
        dt_lowest = x.index[0]
        dt_highest = y.index[0]
        crisis_data = [
            (dt_lowest, '%.2f' % lowest, lowest, -8),
            (dt_highest, '%.2f' % highest, highest, 8)
        ]
        for date, label, y, offset in crisis_data:
            ax1.annotate(label, xy=(date, y),
                         xytext=(date, y+offset),
                         arrowprops=dict(arrowstyle="->", connectionstyle="angle3,angleA=0,angleB=-90"),
                         horizontalalignment='left', verticalalignment='top')

        ax1.legend(loc='best')
        ax1.set_ylim([lowest-10, highest+10])
        ax1.set_title('Gold Price %s - %s' % (self.daily_data.index[0].date(), self.daily_data.index[-1].date()))

    def draw(self):
        fig, (ax0, ax1) = plt.subplots(1,2,figsize=(14,5),tight_layout=True)#,sharey=True)
        self.draw_daily(ax1)
        
        self.xstart = self.current()
        self.xend = self.xstart + 1800
        self.line, = ax0.plot([], [], lw=1)
        self.ax0 = ax0
        self.ax0.set_xlim(self.xstart-1, self.xend+1)
        self.ax0.grid()

        ani = animation.FuncAnimation(fig, self.update, self.gen_data, interval=10000)
        plt.subplots_adjust(wspace=0.1, hspace=None)
        plt.grid(True)
        plt.show()
        #timefmt = DateFormatter('%D %T')
        #self.fig, self.ax = plt.subplots(2,1)
        #self.line, = self.ax[0].plot([])
        #self.ax[0].set_ylim(0, self.ylimit)
        
        #years = HourLocator()   # every year
        #months = MinuteLocator()  # every month
        #timefmt = DateFormatter('%T')
        #self.ax[0].xaxis.set_major_locator(years)
        #self.ax[0].xaxis.set_major_formatter(timefmt)
        #self.ax[0].xaxis.set_minor_locator(months)
        #self.ax[0].autoscale_view()
        #self.ax[0].fmt_xdata = DateFormatter('%T')
        #self.fig.autofmt_xdate()

    def update(self, data):
        print data
        x,y = data
        self.xdata.append(x)
        self.ydata.append(y)

        if x >= self.xend:
            self.xstart += 1800
            self.xend += 1800
            self.ax0.set_xlim(self.xstart-1, self.xend+1)


        self.ax0.set_ylim(min(self.ydata)-0.5, max(self.ydata)+0.5)
        self.line.set_xdata(self.xdata)
        self.line.set_ydata(self.ydata)

        return self.line,

    def gen_data(self, *args):
        while True:
            self.last,output = self.parsehtml(self.last)
            if (self.last['CNY']['bid'] < self.alarm):
                print "\007"
                self.alarm -= 0.5
            if not opt_noout:
                print "\033[2J\033[1;1H"
                print '\033[A'+output

            yield self.last['time'],self.last['CNY']['ask']

    def parsehtml(self, last):
        path = 'http://www.icbc.com.cn/ICBCDynamicSite/Charts/'
        url = path + 'GoldTendencyPicture.aspx'
        fmt = {
            'red': '\033[31m{0}\033[0m',
            'green': '\033[32m{0}\033[0m'
        }
        root = lxml.html.parse(url).getroot()
        table_root = root.find('body/form/table')
        if table_root is None:
            return last,'parse error'
        table_index = 6
        rows = {2:'CNY', 6:'USD'}
        cols = {3:'ask', 4:'bid', 5:'mid', 6:'high', 7:'low'}
        prices = last
        output = ''
        prices['time'] = self.current()

        k = 0
        for tb in table_root.iterfind('tr/td/table'):
            k += 1
            if (k != table_index): continue
            div = tb.find('tbody/tr/td/div')

            row=0
            for tr in div.findall('table/tbody/tr'):
                row += 1
                if (row not in rows.keys()): continue
                output += '%-8s' % (rows[row]) # USD or CNY
                col = 0
                for td in tr.iterchildren():
                    col += 1
                    if (col not in cols.keys()): continue
                    txt = td.text.strip(' \r\n')
                    val = float(txt)
                    align_txt = '%-10s ' % txt
                    if (cols[col] == 'ask'):
                        last_ask = last[rows[row]][cols[col]]
                        if last_ask < val:
                            align_txt = fmt['red'].format(align_txt)
                        elif last_ask > val:
                            align_txt = fmt['green'].format(align_txt)
                    output += align_txt
                    prices[rows[row]][cols[col]] = val

                if rows[row] == 'CNY':
                    output = self.calc_profit(prices) + '\n' + output
                output+='\n'
            return prices,output

    def calc_profit(self, prices):
        if self.number>0 and self.average>0:
            ask = prices['CNY']['ask']
            percentage = (ask - self.average) / self.average * 100
            diff = (ask-self.average)*self.number
            output = 'Date: %s\nVolume:%.1f average:%.3f percentage:%.3f%% profit:%.3f' % (
                prices['time'],
                self.number, self.average, percentage, diff)
            return output
        return ''


def main():
    opt_loop = True
    icbc = ICBC(234.94, 70)

    icbc.draw()


if __name__ == '__main__':
    main()
