#! /bin/bash

cpp_regex=".*\.[hc][pp]*$"

find . -type f -regex $cpp_regex
read -p "Prettify all files (y/n)? " yes
[ "$yes" != "y" ] && exit 0

for i in $(find . -type f -regex $cpp_regex)
do
    echo "Prettify $i"
    bcpp -bcl -i 4 -s $i -fo $i.bcppbak
    if diff $i $i.bcppbak; then
        echo not changed
    else
        cp $i.bcppbak $i
    fi
done
read -p "Delete all backup files (y/n)? " yes
[ "$yes" != "y" ] && exit 0
find . -type f -name "*.bcppbak" | xargs rm -f
