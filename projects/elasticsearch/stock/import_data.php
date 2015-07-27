<?php
require 'vendor/autoload.php';
// http://www.jarloo.com/yahoo_finance/

define("BASE_URI", "http://ichart.finance.yahoo.com/table.csv");
define("INDEX_NAME", "stock");
define("INDEX_TYPE", "sh");

function init()
{
    $params = array();
    $params['connectionPoolClass'] = '\Elasticsearch\ConnectionPool\SniffingConnectionPool';
    $params['hosts'] = array (
        'localhost'
    );

    $params['logging'] = true;
    $params['logPath'] = '/tmp/import_data.log';
    $params['logPermission'] = 0664;
    $params['logLevel'] = Psr\Log\LogLevel::INFO;
    $client = new Elasticsearch\Client($params);

    return $client;
}

function exists($client)
{
    $params = array();
    $params['index'] = INDEX_NAME;
    $response = $client->indices()->exists($params);

    return $response;
}

function create_index($client)
{
    $params['index']  = INDEX_NAME;
    $params['body']['settings']['number_of_shards'] = 3;
    $params['body']['settings']['number_of_replicas'] = 2;

    $properties = json_decode(file_get_contents('stock.mapping'), true);
    $mapping = array(
        '_source' => array(
        'enabled' => true
        ),
        "_timestamp" =>array(
            "enabled" => true,
            "path" => "date"
        ),
        'properties' => $properties
    );
    $params['body']['mappings'][INDEX_TYPE] = $mapping;
    $client->indices()->create($params);
}

function get_symbol($symbol)
{
    $uri = BASE_URI . "?s=$symbol";
    $contents = file_get_contents($uri);
    if (!$contents)
        return [];

    print $uri.PHP_EOL;
    $docs = array_slice(split("\n", $contents), 1, -1);

    return $docs;
}

function bulk($conn, $symbol)
{
    $contents = get_symbol($symbol);
    if (!$contents)
        return false;

    $params['index'] = INDEX_NAME;
    $params['type']  = INDEX_TYPE;

    $count=0;
    $i = 0;
    foreach($contents as $doc) {
        $params['body'][] = array(
            'index' => array(
                '_id' => $i
            )
        );
        list($date,$open,$high,$low,$close,$volume,$adj_close) = split(",", $doc);
        $params['body'][] = array(
            'symbol' => $symbol,
            'date' => date('c', strtotime($date)),
            'open' => $open,
            'close' => $close,
            'high' =>  $high,
            'low' =>  $low,
            'volume' =>  $volume,
            'adj_close' => $adj_close
        );

        $i += 1;
        $count += 1;
        if (($i % 1000) == 0) {
            $responses = $conn->bulk($params);
            if (key_exists('errors', $responses) and $responses['errors'] > 0) {
                print_r($responses['items'][0]);
                exit(0);
            }
            printf("bulk %d records took %d\n", $count, $responses['took']);
            unset($params['body']);
            $count = 0;
        }
    }
    if ($count > 0) {
        $responses = $conn->bulk($params);
        sleep(10);
        printf("final bulk %d records took %d\n", $count, $responses['took']);
    }

    return true;
}

$opt_delete = 0;
$opt_update = 0;
$opt_verbose = 0;
foreach($argv as $a) $GLOBALS["opt_$a"] = 1;

$conn = init();
// Delete Index
$iparams['index'] = INDEX_NAME;
if ($GLOBALS["opt_delete"]) {
    $conn->indices()->delete($iparams);
}
if (!exists($conn)) {
    create_index($conn);
}
bulk($conn, "600036.SS");