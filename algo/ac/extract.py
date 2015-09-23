#! /usr/bin/env python
#coding=utf-8

import sys
import codecs
from sets import Set
from wordseg import WordSegment

def load_file_to_str(filename):
    myfile = codecs.open(filename, encoding='utf-8')
    data = myfile.read().replace('\n', '')
    return data

exclude = [
    u' ',
    u'一',
    u'的'
];


if __name__ == '__main__':
    i = 0
    f = codecs.open(sys.argv[2], encoding='utf-8', mode='a+')

    tx = load_file_to_str(sys.argv[1])
    ws = WordSegment(tx, max_word_len=4, min_aggregation=1, min_entropy=0.5)
    ws.segSentence(tx, WordSegment.L)
    for w in ws.words:
        if any(c in exclude for c in w): continue
        if (w[0] in w[1]): continue
        i += 1
        if i > 10 : break;
        s = u'%s' % w
        print s
        f.write(s+'\n')
    f.close()
