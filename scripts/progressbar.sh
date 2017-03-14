#!/bin/bash
# 1. Create ProgressBar function
# 1.1 Input is currentState($1) and totalState($2)
# @current @total @message

function progressbar {
    # Process data
    let _progress=(${1}*100/${2}*100)/100
    let _done=(${_progress}*4)/10
    let _left=40-$_done

    # Build progressbar string lengths
    _fill=$(printf "%${_done}s")
    _empty=$(printf "%${_left}s")

    printf "\r${3}: [${_fill// /#}${_empty// / }] ${_progress}%%"
}

# Variables
_start=1

# This accounts as the "totalState" variable for the progressbar function
_end=1000

# Proof of concept
for number in $(seq ${_start} ${_end})
do
    sleep 0.1
    progressbar ${number} ${_end} "Decrypting Account.csv"
done
printf '\nFinished!\n'
