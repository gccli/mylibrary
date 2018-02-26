#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import datetime
import calendar

def add_months(sourcedate, months):
    month = sourcedate.month - 1 + months
    year = sourcedate.year + month // 12
    month = month % 12 + 1
    day = min(sourcedate.day,calendar.monthrange(year,month)[1])
    return datetime.date(year,month,day)

if __name__ == '__main__':
    dt = datetime.datetime.strptime(sys.argv[1], '%Y-%m-%d %H:%M:%S')

    print add_months(dt, int(sys.argv[2]))
