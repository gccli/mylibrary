#! /bin/bash

sample_data=$HOME/tmp/samples/data
n=100
max=$(($n*100))
> /tmp/keyword_en
for doc in $(find $sample_data -type f -name *.txt -size +4k)
do
    file -b $doc | grep ASCII >/dev/null
    if [ $? -eq 0 ]; then
        tf=$(mktemp)
        egrep -o '[a-z]{8,20}' $doc | sort | uniq -c | sort -nr | head -$n > $tf
        awk '{ print $2 }' $tf >> /tmp/keyword_en
        unlink $tf
    fi

    i=$(wc -l < /tmp/keyword_en)
    [ $i -gt $max ] && break
    echo extract $i ...
done
sort -u /tmp/keyword_en > /tmp/en_sorted
sed -i '/ffff/d' /tmp/en_sorted
./select_pattern.py /tmp/en_sorted $n | tee /tmp/k_$n
