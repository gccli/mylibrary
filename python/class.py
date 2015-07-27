#! /usr/bin/python

import sys

#class Complex(): # old-style (default)
class Complex(object): # new-style
    """ Complex data structure """
    def __init__(self, real, imag):
        self.r = real
        self.i = imag

    def __str__(self):
        return '%f %f' % (self.r, self.i)
    def __len__(self): 
        return 8

    def __lt__(self, other):
        return self.r < other.r
    def __gt__(self, other):
        return self.r > other.r

    def __eq__(self, other):
        return self.r == other.r and self.i == other.i

def learn_class():
    x = Complex(3,4)
    y = Complex(5,6)
    print 'Demonstrate Complex class:', x.__doc__
    print 'type of x', type(x), 'length:', len(x)
    print 'x.__class__', x.__class__

    print 'x=', x
    print 'y=', y
    
    print 'x < y', x<y
    print 'x == y', x==y

learn_class()





