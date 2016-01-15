Nginx Develop
=============

Nginx Enable PHP
----------------



[http://wiki.jikexueyuan.com/project/nginx/]()
[http://www.evanmiller.org/nginx-modules-guide-advanced.html]()
[http://www.evanmiller.org/nginx-modules-guide.html]()


sudo apt-get install php5-common php5-cli php5-fpm


## 编写Nginx模块 ##

    ngx_addon_name=ngx_http_ai_module
    HTTP_MODULES="$HTTP_MODULES ngx_http_ai_module"
    NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_appid_module.c"
    HTTP_INCS="$HTTP_INCS $ngx_addon_dir"

## 透明代理 ##


https://help.ubuntu.com/community/NetworkConnectionBridge
