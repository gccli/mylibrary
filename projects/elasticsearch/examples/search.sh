#! /bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 filename"
    exit 0
fi

URI=http://localhost:9200/result_*/_search?pretty
curl -i -XGET $URI -d @$1
