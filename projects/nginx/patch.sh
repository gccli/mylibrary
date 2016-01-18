#! /bin/bash

if [ ! -d upload ]; then
    [ ! -f nginx_upload_module-2.2.0.tar.gz ] &&\
        wget http://www.grid.net.ru/nginx/download/nginx_upload_module-2.2.0.tar.gz
    tar xzvf nginx_upload_module-2.2.0.tar.gz
fi
mv nginx_upload_module-2.2.0 upload
cd upload
patch ngx_http_upload_module.c ../davromaniak.txt
