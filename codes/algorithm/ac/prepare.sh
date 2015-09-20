#! /bin/bash

cwd=$PWD
total=60
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
        file -b $doc | grep ASCII
        if [ $? -eq 0 ]; then
            egrep -o '[a-z]{8,20}' $doc >> /tmp/keyword_en
            cp $doc /tmp/data
        fi
    done
    sort -u /tmp/keyword_en > /tmp/en_sorted
fi

if [ ! -f /tmp/keyword_cn ]; then
    cd wordseg
    cp $cwd/extract.py ./
    for doc in $(find $docpath/zh_CN -type f)
    do
        file -b $doc | grep text
        if [ $? -eq 0 ]; then
            cp $doc /tmp/data
            ./extract.py $doc /tmp/keyword_cn
        fi
    done
    sort -u /tmp/keyword_cn > /tmp/cn_sorted
    cd ..
fi

fc=$(ls /tmp/data | wc -l)
cp $cwd/select_pattern.py ./
chmod +x select_pattern.py
cn=$(wc -l < /tmp/cn_sorted)
en=$(($total-$cn))
./select_pattern.py /tmp/en_sorted $en | tee /tmp/keyword
#cat /tmp/cn_sorted >> /tmp/keyword

echo
echo "chinese keyword: $cn"
echo "english keyword: $en"
echo "file count: $fc"
echo "keyword count $(wc -l /tmp/keyword)"
file /tmp/keyword
