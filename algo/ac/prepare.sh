#! /bin/bash

cwd=$PWD
pt_cnt_en=100
pt_cnt_cn=00
cd ~/tmp

echo "================ prepare test data... ================"

if [ "$1" == "clean" ]; then
    rm -f /tmp/keyword* /tmp/*sorted
    rm -rf /tmp/data
fi
mkdir -p /tmp/data
if [ ! -f linux-4.2.tar.xz ]; then
    wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.2.tar.xz
fi
if [ ! -d ~/tmp/linux-4.2 ]; then
    tar xxf linux-4.2.tar.xz
fi
if [ ! -d wordseg ]; then
    git clone https://github.com/Moonshile/ChineseWordSegmentation.git wordseg
fi

docpath=$PWD/linux-4.2/Documentation

if [ ! -f /tmp/keyword_en ]; then

    for doc in $(find $docpath -type f -size +4k)
    do
        file -b $doc | grep ASCII >/dev/null
        if [ $? -eq 0 ]; then
            #echo "$doc top 10 word"
            tmp=$(mktemp)
            #egrep -o '[a-z]{8,20}' $doc | sort | uniq -c | sort -nr | head | tee $tmp
            egrep -o '[a-z]{8,20}' $doc | sort | uniq -c | sort -nr | head > $tmp
            awk '{ print $2 }' $tmp >> /tmp/keyword_en
            unlink $tmp
            cp $doc /tmp/data
        fi
    done
    sort -u /tmp/keyword_en > /tmp/en_sorted
    for doc in $(find /usr/share/doc -type f -size +4k)
    do
        cp $doc /tmp/data
    done
fi

if [ ! -f /tmp/keyword_cn ]; then
    cd wordseg
    cp $cwd/extract.py ./
    for doc in $(find $docpath/zh_CN -type f)
    do
        file -b $doc | grep text >/dev/null
        if [ $? -eq 0 ]; then
            cp $doc /tmp/data
            echo "$doc top 3 word"
            ./extract.py $doc /tmp/keyword_cn
        fi
    done
    sort -u /tmp/keyword_cn > /tmp/cn_sorted
    cd ..
fi

fc=
cp $cwd/select_pattern.py ./

sed -i '/ffff/d' /tmp/en_sorted
./select_pattern.py /tmp/en_sorted $pt_cnt_en | tee /tmp/keyword
if [ $pt_cnt_cn -gt 0 ]; then
    ./select_pattern.py /tmp/cn_sorted $pt_cnt_cn | tee -a /tmp/keyword
fi
echo
echo "total file count: $(ls /tmp/data | wc -l)"
echo "keyword count:$(wc -l < /tmp/keyword), cn:$pt_cnt_cn, en:$pt_cnt_en"
echo "keyword file format: $(file /tmp/keyword)"
