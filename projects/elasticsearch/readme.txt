* Document APIs
** Index/Get/Delete
   curl -XPUT "http://localhost:9200/$my_index/$my_type/$my_id"
   versioning: Each indexed document is given a version number. The associated version number is returned as part of the response to the index API request.
   curl -XGET "http://localhost:9200/$my_index/$my_type/$my_id"
   curl -XGET "http://localhost:9200/$my_index/$my_type/$my_id?_source=false&pretty"
   curl -XDELETE "http://localhost:9200/$my_index/$my_type/$my_id"
** Update
   update a document based on a script provided. It uses versioning to make sure no updates have happened during the "get" and "reindex".
   curl -XPOST "localhost:9200/$my_test/$my_type1/$my_id/_update" -d '{
    "script" : "ctx._source.counter += count",
    "params" : {
        "count" : 4
    }
  }'

** Multi Get
  curl "http://localhost:9200/$my_index/$my_type/_mget" -d '{
     "ids" : ["1", "2"]
  }'
** Bulk
   perform many index/delete operations. endpoints are /_bulk, /{index}/_bulk, and {index}/{type}/_bulk
   + action_and_meta_data\n
     optional_source\n
     action_and_meta_data\n
     optional_source\n
     ....
     action_and_meta_data\n
     optional_source\n
     { "index" : { "_index" : "test", "_type" : "type1", "_id" : "1" } }
     { "field1" : "value1" }
     { "delete" : { "_index" : "test", "_type" : "type1", "_id" : "2" } }
     { "create" : { "_index" : "test", "_type" : "type1", "_id" : "3" } }
     { "field1" : "value3" }
     { "update" : {"_id" : "1", "_type" : "type1", "_index" : "index1"} }
     { "doc" : {"field2" : "value2"} }
   curl -s -XPOST "http://localhost:9200/$my_index/$my_type/_bulk" --data-binary @requests

** Term Vectors
   curl -s -XPOST "http://localhost:9200/$my_index/$my_type/$my_id/_termvertor?pretty"

* Indices APIs
** Get/Exist/Status/Stats
   curl -XGET "http://localhost:9200/$my_index?pretty"
   curl -XHEAD -i "http://localhost:9200/$my_index"
   curl -XGET "http://localhost:9200/$my_index/_status?pretty"
   curl -XGET "http://localhost:9200/_stats?pretty"

* cat APIs - such as show indices
** Parmeters
*** Help
    curl 'localhost:9200/_cat/indices?help
*** Headers
    curl 'localhost:9200/_cat/nodes?h=ip,port,heapPercent,name'
    curl 'localhost:9200/_cat/indices/event_*?h=i' # show all indices based on name pattern
** Show Indices
*** show all indices
    curl "http://localhost:9200/_cat/indices?v"
*** show indices with specific patterns
    curl "http://localhost:9200/_cat/indices/event_*?v"

* Cluster
  curl -XGET "http://localhost:9200/_cluster/health?pretty=true"

* Search APIs
** [[https://www.elastic.co/guide/en/elasticsearch/reference/current/search-uri-request.html][URI Search]]
   ...

* Query DSL
** query and filter
   + Queries should be used instead of filters:
     for full text search, and where the result depends on a relevance score
   + Filters should be used instead of queries:
     for binary yes/no searches and for queries on exact values
** Queries

** Filters


** Controlling Relevance
   [[https://www.elastic.co/guide/en/elasticsearch/guide/current/relevance-intro.html][+ What is Relevance?]]
     Term frequency: How often does the term appear in the field
     Inverse document frequency: How often does each term appear in the index
     Field-length norm: A term appearing in a short /title/ field carries more weight than a long /content/ field
   布尔模型(Boolean Model)
   词条频度/倒排文档频度(TF/IDF) - Term Frequency/Inverse Document Frequency
   Query-time boosting is the main tool that you can use to tune relevance.

   curl -i -XGET 'http://localhost:9200/_search?explain&pretty' '{"query"   : { "match" : { "name" : "Jeck" }}}'
