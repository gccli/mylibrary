#!/bin/bash

data='{"type":1,"vendor":2,"appid_action":265,"datetime":1462424508,"protocol":2,"userid":"gaoxuefeng@360.cn","URL":"ws360.my.salesforce.com\/800\/o","source":"10.18.240.132"}'
for i in `seq 1 100`;
do
    port=$((10086+$i%2))
    echo $data | nc -q1 -u 127.0.0.1 $port &
done
