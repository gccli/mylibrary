#! /usr/bin/env python
#coding=utf-8

import sys
import codecs
from sets import Set
from wordseg import WordSegment


exclude = [
    u' ',
    u'一',
    u'的'
];

words = Set()
def scan_file(filename):
    i = 0
    ext_cnt = 10
    myfile = codecs.open(filename, encoding='utf-8')
    while True:
        tx = myfile.read(102400)
        if not tx:
            break;
        if (len(tx) > 100000): ext_cnt=20

        ws = WordSegment(tx, max_word_len=4, min_aggregation=20, min_entropy=1.0)
        ws.segSentence(tx, WordSegment.L)
        for w in ws.words:
            if any(c in exclude for c in w): continue
            i += 1
            if i > ext_cnt : break;
            s = u'%s\n' % w
            words.add(s)
        break;
    myfile.close()

if __name__ == '__main__':
    scan_file(sys.argv[1])
    with codecs.open(sys.argv[2], "a+", encoding='utf-8') as f:
        for w in words:
            f.write(w)
        f.close()
