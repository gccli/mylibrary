echo 'search...'
curl 'http://localhost:9200/event_*/log/_search?pretty' -d '
{
  "fields": ["_id"],
  "query": {
    "match": {
      "appid_action": {
        "query": "Unknown 25",
        "type": "phrase"
      }
    }
  }
}
' | tee /tmp/search.json

echo
echo delete by query

ids=$(grep _id /tmp/search.json  | egrep -o '[0-9]+')
for i in $ids
do
    echo DELETE /event_20160516/log/$i
    curl -XDELETE http://localhost:9200/event_20160516/log/$i
done
