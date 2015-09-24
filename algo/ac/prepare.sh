#! /bin/bash

sample_orig=/root/tmp/samples/orig
sample_prep=/root/tmp/samples/prepare
sample_data=/root/tmp/samples/data

# original -> prepare -> data
function copyalldoc() {
    dir=$1
    dst=$2

    for i in $(find $dir -type f)
    do
        cp $i $dst/
    done
}

cwd=$(pwd)
pt_cnt_en=800
pt_cnt_cn=200

echo "================ Prepare test data... ================"
cd $sample_prep
if [ "$1" == "clean" ]; then
    rm -f keyword* *sorted
fi
linuxdoc=$sample_prep/linux-4.2/Documentation

mkdir -p $sample_data
if [ ! -d linux-4.2 ]; then
    echo "==== Decompress linux kernel ..."
    tar xxf $sample_orig/linux-4.2.tar.xz
    [ $? -ne 0 ] && exit 2
fi
if [ ! -d wordseg ]; then
    echo "==== Download WordSegmentation ..."
    git clone https://github.com/Moonshile/ChineseWordSegmentation.git wordseg
fi
if [ ! -d rfc ]; then
    echo "==== Decompress RFC documents ..."
    mkdir -p rfc
    tar -C rfc -xzf $sample_orig/RFC-all.tar.gz
    [ $? -ne 0 ] && exit 2
fi

if [ ! -d  cn ]; then
    mkdir -p cn
    copyalldoc $sample_orig/cn cn
    copyalldoc $linuxdoc/zh_CN cn
fi

if [ ! -f keyword_en ]; then
    echo "==== Copy files linux kernel doc ..."
    for doc in $(find $linuxdoc -type f -size +4k)
    do
        file -b $doc | grep ASCII >/dev/null
        if [ $? -eq 0 ]; then
            tf=$(mktemp)
            egrep -o '[a-z]{8,20}' $doc | sort | uniq -c | sort -nr | head > $tf
            awk '{ print $2 }' $tf >> $sample_prep/keyword_en
            unlink $tf
            cp $doc $sample_data
        fi
    done
    sort -u $sample_prep/keyword_en > $sample_prep/en_sorted
    sed -i '/ffff/d' $sample_prep/en_sorted

    echo "==== Copy files RFCs ..."
    cp rfc/*.txt $sample_data
fi

if [ ! -f keyword_cn ]; then
    cd wordseg
    cp $cwd/extract.py ./
    for doc in $(find $sample_prep/cn -type f)
    do
        file -b $doc | grep text >/dev/null
        if [ $? -eq 0 ]; then
            cp $doc $sample_data
            echo "extract $doc ..."
            ./extract.py $doc $sample_prep/keyword_cn
        fi
    done
    sort -u $sample_prep/keyword_cn > $sample_prep/cn_sorted

    cd ..
fi


cp $cwd/select_pattern.py ./

./select_pattern.py $sample_prep/en_sorted $pt_cnt_en | tee /tmp/keyword
if [ $pt_cnt_cn -gt 0 ]; then
    ./select_pattern.py $sample_prep/cn_sorted $pt_cnt_cn | tee -a /tmp/keyword
fi
echo
echo "total file count: $(ls $SAMPLE_DATE | wc -l)"
echo "keyword count:$(wc -l < /tmp/keyword), cn:$pt_cnt_cn, en:$pt_cnt_en"
echo "keyword file format: $(file /tmp/keyword)"
