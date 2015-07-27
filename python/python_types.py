#! /usr/bin/python

import sys
import random
import math
def showtypes():
    """ Show variables types """

    myids = None
    print 'type of', 'None :', type(myids)
    myids = True
    print 'type of', 'True :', type(myids)
    myids = 0x0fffffff
    print 'type of', '0x0fffffff:', type(myids)
    myids = 0xfffffffffffffffffffffffffff
    print 'type of', '0xfffffffffffffffffffffffffff:', type(myids)
    myids = math.pi
    print 'type of', 'PI:', type(myids)

    myids = 'hello'
    print 'type of', myids, type(myids)
    myids = u'hello'
    print 'type of', myids, type(myids)

    myids = ('1', math.pi, '2', 3, 'hello world', b'0x')
    print 'type of (...):', type(myids)
    myids = {'guido': 4127, 'irv': 4127, 'jack': 4098}
    print 'type of [...]:', myids, type(myids), 'len:', len(myids)
    myids = {'1':'char 1', 1:'number 1', '2':'char 2', 3:'number 3', 4:'number 4'}
    print 'type of {...}:', type(myids)
    myids = bytearray('xxxxxxxxxx')
    print 'type of bytearray(\'xxxxxxxxxx\')',  type(myids)

    print 'type of showtypes', type(showtypes), 'doc', __name__
    print

    array = ['pi', math.pi, (5**0.5-1)/2, 2**0.5]
    print array


showtypes()

