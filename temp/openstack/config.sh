#! /bin/bash

function ini_replace {
    local s=$1 #section
    local p=$2 #pattern
    local l=$3 #replace line
    local f=$4 #filename
    local o='-i.bak' # option

    cp $f /tmp/$(basename $f)

    echo sed $o "/^\[$s\]/,/^\[/{s/$p/$l/}" $f
    sed $o "/^\[$s\]/,/^\[/{s/$p/$l/}" $f
    [ $? -ne 0 ] && return 1

    diff $f /tmp/$(basename $f)

    return 0
}

ini_replace DEFAULT '^[#]*rpc_backend.*' 'rpc_backend = iiiiiii' nova.conf