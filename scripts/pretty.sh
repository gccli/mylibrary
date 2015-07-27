#! /bin/bash

tmp=$(mktemp -d)
echo "Create tmp directory - $tmp"
for i in $@
do
    echo "Prettify $i"
    cp -f $i $tmp/
    bcpp -bcl -i 2 -s $i -fo $i.1
    mv $i.1 $i
done

read -p "Prettify all files (y/n)? " yes
if [ "$yes" != "y" ]; then
    exit 0
fi

for i in `ls *.h *.cpp`
do
    echo "Prettify $i"
    cp -f $i $tmp/
    bcpp -bcl -i 2 -s $i -fo $i.1
    mv $i.1 $i
done
