#! /bin/bash

curl -XDELETE localhost:9200/test
curl -XPOST localhost:9200/test -d '{
 "settings" : {
   "number_of_shards" : 1
 },
 "mappings" : {
    "type1" : {
       "_source" : { "enabled" : true },
       "properties" : {
          "field1" : { "type" : "string", "index" : "not_analyzed" },
          "field2" : { "type" : "nested", "properties": {"first" : {"type": "string" },"last"  : {"type": "string" }} }
        }
    }
  }
}'
echo

curl -s -XPOST localhost:9200/_bulk --data-binary @requests; echo
sleep 1
curl localhost:9200/test/_search?pretty; echo 
echo 'Mapping'
curl -XGET 'http://localhost:9200/test/_mapping?pretty'