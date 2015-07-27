#! /bin/bash

wget http://downloads.mysql.com/docs/world_innodb.sql.gz
gzip -d world_innodb.sql.gz 

mysql <<EOF
drop database if exists world;
create database world;
use world
source world_innodb.sql
quit
EOF


echo 
echo install repl.users

mysql < users.sql

TB_NAME=users;

mysql -e "truncate table repl.$TB_NAME"

function insert()
{
    for i in `seq 1 300`
    do
	mysql -e "insert into repl.$TB_NAME(username,domain,password,home) values($i, 'inetlinux.net', MD5('cc'), '/home/$i');"
    done
}

function selectusers()
{
    mysql -e "select * from repl.$TB_NAME"
}

time insert
time selectusers

