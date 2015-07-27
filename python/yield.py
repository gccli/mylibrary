#! /usr/bin/env python

# understanding yield
# http://www.ibm.com/developerworks/cn/opensource/os-cn-python-yield/

import sys

def fab(nmax):
    n,a,b = 0,0,1
    while n < nmax:
        yield b
        a,b=b,a+b
        n=n+1


for i in fab(16):
    print '  %d' % i

def read_file(fpath): 
    BLOCK_SIZE = 1024 
    with open(fpath, 'rb') as f: 
        while True: 
            block = f.read(BLOCK_SIZE) 
            if block: 
                yield block 
            else: 
                return


if __name__ == '__main__':
    print 'reading file', sys.argv[1]
    for block in read_file(sys.argv[1]):
        print '  block length ', len(block)

