#!/bin/bash

# Program to print a text file with headers and footers

function alarm_headler {
    local c=0
    while [ $c -lt 4 ]
    do
	usleep 500000
	echo -ne "."
	c=$(($c+1))
    done
    echo -ne "\033[0J\033[1K\033[4D"
    kill -ALRM $$ 
}

trap alarm_headler SIGALRM
kill -ALRM $$ 

while true
do
    ls /tmp > /dev/null
done




