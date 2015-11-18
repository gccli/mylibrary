#! /bin/bash

verbose=1
enc=/tmp/enc
dec=/tmp/dec

function symenc()
{
ciphers="aes-128-cbc aes-256-cbc bf bf-cbc bf-cfb des des-cbc des3 rc2 rc4"
for c in $(echo $ciphers)
do
    for inf in $(find sample -type f)
    do
        ./crypt $inf $enc -c $c
        [ $? -ne 0 ] && exit 1

        ./crypt $enc $dec -c $c -d
        [ $? -ne 0 ] && exit 1

        s1=$(stat -c%s $inf)
        s2=$(stat -c%s $enc)
        s3=$(stat -c%s $dec)
        [ $s1 -ne $s3 ] && echo 'error in size' && exit 1
        diff $inf $dec || (echo 'diff error' && exit 1)
        echo -e "ok\n"
    done
done
}

function asymenc()
{
    for inf in $(find ./ -type f)
    do
        rm -f $enc $dec
        #using public key encrypt
        echo -e "./crypt $inf $enc"
        ./crypt $inf $enc -p
        [ $? -ne 0 ] && exit 1

        #using private key decrytp
        echo -e "./crypt $enc $dec -d"
        ./crypt $enc $dec -d
        [ $? -ne 0 ] && exit 1

        s1=$(stat -c%s $inf)
        s2=$(stat -c%s $enc)
        s3=$(stat -c%s $dec)
        [ $s1 -ne $s3 ] && echo 'error in size' && exit 1
        diff $inf $dec || (echo 'diff error' && exit 1)
        echo -e "ok\n"
    done
}

asymenc
