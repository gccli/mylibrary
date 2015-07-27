#! /bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 filename"
    exit 0
fi

URI=http://localhost:9200/driver/_search?pretty
CMD="curl -i -XGET $URI -d @$1"
echo $CMD
$CMD
