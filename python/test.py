#! /usr/bin/python

import sys

print sys.argv

x=33

if (x%2 == 0):
    print 'x =', x
    print 'x is even'
else:
    print 'x =', x
    print 'x is odd'


x,y,z=14,11,10

if (x < y and x < z):
    print 'x is the smallest'
elif (y < z):
    print 'y is the smallest'
else :
    print 'z is the smallest'

#parents, babies = (1, 1)
#while babies < 1000:
#    print 'This generation has %d babies' % babies
#    parents, babies = (babies, parents + babies)


n=30
parents, babies = (0, 1)
for i in range(1, n+1):
    parents, babies = (babies, parents + babies)
print 'This generation has %d babies' % babies

def fibonacci(n):
    '''fibonacci function'''
    if (n == 0):
        return 0;
    elif (n == 1):
        return 1;

    x = 0;
    y = 1;
    fib = 0;
    for i in range(n):
        fib = x+y;
        x = y;
        y = fib;
    return y;

def ispalindrome(s):
    if len(s) <= 1:
        return True
    else: return s[0] == s[-1] and ispalindrome(s[1:-1])

print(fibonacci(30))

print(ispalindrome('sys'))
print(ispalindrome('system'))
