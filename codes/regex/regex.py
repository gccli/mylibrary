#! /usr/bin/env python

import re
import sys

regex_patterns = [
    'feedItemId":"([^"]+)'
]

s = open(sys.argv[1], 'r').read()
for r in regex_patterns:
    m = re.search(r, s)
    if not m: continue
    print m.group(0)
    for g in m.groups(): print '  ', g
    print '------'
