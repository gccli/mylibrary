#! /usr/bin/env python

import sys
import random

if __name__ == '__main__':
    hex = ['aaa','bbb','ccc','ddd','eee','fff']
    pts = []
    lines = []
    with open(sys.argv[1]) as f:
        while True:
            lines = f.readlines(100)
            if len(lines) < 100: break
            for i in range(0, len(lines), 50):
                w = random.choice(lines)
                if any(h in w for h in hex): continue
                pts.append(w)
        random.shuffle(pts)
        f.close()

    slic=pts[:int(sys.argv[2])]
    for s in slic:
        print s,
