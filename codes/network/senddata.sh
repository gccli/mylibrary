#! /bin/sh


for i in `seq 1 100`
do
  n=$((100+$RANDOM%500))
  ./i6udp -ch localhost6 -p3200 -l $n &
  ./i6udp -ch ff10::e100:1 -p3200 -l $n &
done