#! /bin/bash

cwd=$PWD

cd ~/tmp
rm -rf /tmp/data
mkdir /tmp/data

echo "================ prepare test data... ================"

if [ ! -f linux-4.2.tar.xz ]; then
    wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.2.tar.xz
fi

if [ ! -d ~/tmp/linux-4.2 ]; then
    tar xxf linux-4.2.tar.xz
fi

docpath=$PWD/linux-4.2/Documentation
#for doc in $(find $docpath -type f -size +4k)
#do
#    file $doc | grep text
#    if [ $? -eq 0 ]; then
#        cp $doc /tmp/data
#    fi
#done


if [ ! -d wordseg ]; then
    git clone https://github.com/Moonshile/ChineseWordSegmentation.git wordseg
fi
cp $cwd/extract.py wordseg
cd wordseg
> /tmp/keyword_cn
for doc in $(find $docpath/zh_CN -type f)
do
    file $doc | grep text
    if [ $? -eq 0 ]; then
        cp $doc /tmp/data
        ./extract.py $doc /tmp/keyword_cn
    fi
done

sort -u /tmp/keyword_cn > /tmp/keyword
