#! /usr/bin/env python

import sys
import random

if __name__ == '__main__':
    pts = []
    with open(sys.argv[1]) as f:
        pts = f.readlines()
        random.shuffle(pts)
        f.close()

    slic=pts[:int(sys.argv[2])]
    for s in slic:
        print s,
