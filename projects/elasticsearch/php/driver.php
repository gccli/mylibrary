<?php
require 'vendor/autoload.php';

define("INDEX_NAME", "driver");
define("INDEX_TYPE", "driver");

define("BEIJING", "39.9084974220,116.3974337341");
define("SHANGHAI", "31.2433145019,121.4811272757");
define("SHENZHEN", "22.5399477530,114.0194855048");
define("URUMCHI", "43.8346112822,87.5584165186");

function init()
{
    $params = array();
    $params['connectionPoolClass'] = '\Elasticsearch\ConnectionPool\SniffingConnectionPool';
    $params['hosts'] = array (
        'localhost'
    );

    $params['logging'] = true;
    $params['logPath'] = '/tmp/elasticsearch.log';
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
    $params['body']['settings']['number_of_replicas'] = 1;

    $properties = json_decode(file_get_contents('driver.mapping'), true);
    $mapping = array(
        'properties' => $properties
    );
    $params['body']['mappings'][INDEX_TYPE] = $mapping;
    $ret = $client->indices()->create($params);
}

function random($min = 0.0, $max = 1.0) {
    return $min + mt_rand() / mt_getrandmax() * ($max - $min);  
}

function location($center, $min = -0.3, $max = 0.3) {
    list($lat,$lon) = split(',', $center);
    $lat += random($min, $max);
    $lon += random($min, $max);

    return sprintf("%f,%f", $lat, $lon);
}

function time_range($hour, $interval) 
{
    $minute = rand() % 60;
    $today = date("Y,m,d");
    list($y,$m,$d) = split(',', $today);
    $begin = date("H:i:s", mktime($hour, $minute, 0, $m, $d, $y));
    $minute += $interval;
    while  ($minute > 60) {
        $minute -= 60;
        $hour += 1;
    }
//    $end = date("Y-m-d H:i:s", mktime($hour, $minute, 0, $m, $d, $y));
    $end = date("H:i:s", mktime($hour, $minute, 0, $m, $d, $y));
    return array($begin, $end);
}

function time_slot($time, $mode='round')
{
    list($h,$m,$s) = split(':', $time);
    $slot = 0;
    $interval = 30;
    if ($mode == 'floor') {
        $slot = floor($m/$interval);
    } else if ($mode == 'ceil') {
        $slot = ceil($m/$interval);
    } else {
        $slot = round($m/$interval);
    }
    
    return (int)(2*($h-1) + $slot);
}

function time_slot_range($start, $end)
{
    return range(time_slot($start), time_slot($end));
}

function bulk($client)
{
    $names = [];
    $contents = split("\n", file_get_contents('names.txt'));
    foreach ($contents as $n) {
        if (strlen($n) < 10) continue;
        $names[] = split("[ \t]", $n)[0];
    }
    $name_size = count($names);

    $descs = [];
    $contents = split("\n", file_get_contents('elasticsearch.txt'));
    foreach ($contents as $n) {
        if (strlen($n) < 10) continue;
        $descs[] = split("\n", $n)[0];
    }
    $desc_size = count($descs);

//    $free_slots = time_slot_range('07:00:00', '21:00:00');
    $schedules = array_merge(time_slot_range('07:00:00', '12:00:00'), time_slot_range('13:00:00', '16:00:00'));

    $total = 10000;
    $count = 0; // inserted count
    $params['index'] = INDEX_NAME;
    $params['type']  = INDEX_TYPE;

    for($i = 0; $i < $total;) {
        $params['body'][] = array(
            'index' => array('_id' => $i)
        );
        $randi = rand(0, 7200);
        $center = BEIJING;
        $city = '北京';
        if ($randi > 3600 and $randi <= 4800) {
            $center = SHANGHAI;
            $city = '上海';
        } else if ($randi > 4800 and $randi <= 6600) {
            $center = SHENZHEN;
            $city = '深圳';
        } else if ($randi > 6600) {
            $center = URUMCHI;
            $city = '乌鲁木齐';
        }

        $sched1 = time_range(rand(7, 11), rand(120, 360));
        $sched2 = time_range(rand(13, 16), rand(30, 90));
        $busy_slots = array_merge(time_slot_range($sched1[0], $sched1[1]),
                                  time_slot_range($sched2[0], $sched2[1]));

        $params['body'][] = array(
            'name' => $names[$i % $name_size],
            'cartype' => $randi%4 + 1,
            'location' => location($center),
            'city' => $city,
            'update' => date('c', time()-$randi),
            'schedule' => array('slot' => $schedules),
            'agenda' => array(
                array('start' => $sched1[0], 'end' => $sched1[1],
                'start_point' => location($center, -0.1, 0.1),
                'end_point' => location($center, -0.1, 0.1)
                ),
                array('start' => $sched2[0], 'end' => $sched2[1],
                'start_point' => location($center, -0.1, 0.1),
                'end_point' => location($center, -0.1, 0.1),
                )
            ),
            'busy' => array('slot' => $busy_slots)
        );
        if ($i < $desc_size) {
            $params['body'][2*$i+1]['desc'] = $descs[$i];
        }
        $i++;
        $count++;
        if (($i % 2000) == 0) {
            $responses = $client->bulk($params);
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
        $responses = $client->bulk($params);
        if (key_exists('errors', $responses) and $responses['errors'] > 0) {
            print_r($responses['items'][0]);
            exit(0);
        }
        printf("Final bulk %d records took %d\n", $count, $responses['took']);
    }
}

function search($client, $json)
{
    $params['index'] = INDEX_NAME;
    $params['type']  = INDEX_TYPE;
    $params['body'] = $json;

    printf(">>>\n");
    if ($GLOBALS["opt_verbose"] > 0) {
        printf("-------- Query Request --------\n");
        var_dump($json);
    }
    $results = $client->search($params);
    printf("Total hit count %d, took %d, top 5 as follow\n", 
      $results['hits']['total'], $results['took']);

    if ($GLOBALS["opt_verbose"] > 0) {
        print_r(json_encode($results, JSON_PRETTY_PRINT));
    }
    foreach ($results['hits']['hits'] as $hit) {
        $s = $hit['_source'];
        printf("name:%-10s cartype:%d location:(%s) distance:%.3fkm\n", 
          $s['name'], $s['cartype'], $s['location'], 
        key_exists('sort', $hit) ? $hit['sort'][0] : 0.0);
    }
    printf("<<<\n");
}

$opt_delete = 0;
$opt_update = 0;
$opt_verbose = 0;
foreach($argv as $a) $GLOBALS["opt_$a"] = 1;

$conn = init();
// Delete Index
$iparams['index'] = INDEX_NAME;
if ($GLOBALS["opt_delete"]) {
    if (exists($conn)) $conn->indices()->delete($iparams);
}
if ($GLOBALS["opt_update"]) {
    bulk($conn);
}

if (!exists($conn)) {
    create_index($conn);
    $ret = $conn->indices()->getMapping($iparams);
    printf("Index mapping:\n");
    printf("%s\n", json_encode($ret, JSON_PRETTY_PRINT));
    bulk($conn);
}

// Search
printf("获取“中国技术交易大厦”附近1公里内的5个位置\n");
$json = file_get_contents('geo_distance.json');
search($conn, $json);

printf("获取“中国技术交易大厦”1-3公里之间的5个位置\n");
$json = file_get_contents('geo_distance_range.json');
search($conn, $json);

printf("在“西苑”和“人民大学”间找5个位置\n");
$json = file_get_contents('geo_bounding_box.json');
search($conn, $json);

printf("在“西苑”和“人民大学”间找到“中国技术交易大厦”最近的5个位置\n");
$json = file_get_contents('sort_by_distance.json');
search($conn, $json);

$json = file_get_contents('search_optimum');
search($conn, $json);
?>
