#! /bin/bash


if [ -n "$1" ]; then
    unset OS_AUTH_URL OS_TENANT_NAME OS_USERNAME OS_PASSWORD
else
    export OS_AUTH_URL=http://localhost:5000/v2.0
    export OS_TENANT_NAME=admin
    export OS_USERNAME=admin
    export OS_PASSWORD=cc
    #export OS_AUTH_TOKEN=
fi
