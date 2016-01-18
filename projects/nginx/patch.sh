#! /bin/bash


rm -rf upload
[ ! -f nginx_upload_module-2.2.0.tar.gz ] &&\
    wget http://www.grid.net.ru/nginx/download/nginx_upload_module-2.2.0.tar.gz

tar xzvf nginx_upload_module-2.2.0.tar.gz
mv nginx_upload_module-2.2.0 upload
cd upload
patch ngx_http_upload_module.c ../davromaniak.txt


sed -i '2625,2668d' ngx_http_upload_module.c
sed -i '245d' ngx_http_upload_module.c
