#! /bin/bash

INF=cipher.h
for INF in $(find . -type f)
do
    ./crypt -i $INF -o /tmp/enc.bin
    ls -l $INF /tmp/enc.bin
    ./crypt -i /tmp/enc.bin -o /tmp/dec.bin -d
    diff $INF /tmp/dec.bin || (echo 'error' && exit 1)
done
