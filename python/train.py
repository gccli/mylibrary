#! /usr/bin/env python

import re
import os
import sys
import string

def usage(proc):
    print 'Usage: %s number carriage'
    sys.exit(0)

class Carriage(object):
    seats_total=0
    line_start=0
    line_normal=0
    group_start=0
    group_normal=0
    def __init__(self, args=()):
        self.seats_total,self.line_start,self.line_normal,self.group_start,self.group_normal = args

class Train(object):
    number = ''
    carriages = {}
    def __init__(self, num):
        self.number = num

    def add(self, num, carr):
        self.carriages[num] = carr

    def get(self, num):
        return self.carriages[num]

class TrainsDB(object):
    trains = {}
    trains_data = {
        'T5688':{14:(118,2,5,4,10), 15:(118,5,5,10,10)}
        }
    def __init__(self,filename=''):
        for k,v in self.trains_data.items():
            self.add(k,v)

    def add(self, code, data):
        self.trains[code]=Train(code)
        for n,c in data.items():
            self.trains[code].add(n, Carriage(c))

    def get(self, code):
        return self.trains[code]

if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage(sys.argv[0])
    tdb = TrainsDB()
    train=tdb.get(sys.argv[1])
    carriage=train.get(int(sys.argv[2]))

    left=2
    line_num=carriage.line_start
    line_i=0
    grp_num=carriage.group_start
    grp_i=0

    print '++++++++++++++++++++++++'
    for seat in xrange(0, carriage.seats_total):
        print '%3d' % (seat+1),
        grp_i += 1
        line_i += 1
        if (grp_i == grp_num):
            print '\n+----------------------+'
            grp_i = 0
        if (line_i == line_num):
            line_i=0
        if (line_i==left):
            print ' || ',
        if (grp_i == line_num):
            print ''
        if (seat >= carriage.group_start):
            grp_num=carriage.group_normal
            line_num=carriage.line_normal
    print '\n++++++++++++++++++++++++'
