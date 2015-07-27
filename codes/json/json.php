<?php


$json = file_get_contents("/tmp/config.json");
$obj = json_decode($json, true);
if ($obj == NULL) 
    print "null object\n";
else
    print_r($obj);

print "\n--------------------------------\n";
$json = '{ "host": "10.0.11.224", "port": "3306", "username": "yongche" }';
$obj = json_decode($json, true);
print_r($obj);

print "\n--------------------------------\n";
$json = '["172.16.0.135:27017","172.16.0.136:27017","172.16.0.137:27017"]';
$obj = json_decode($json, true);
var_dump($obj);
print_r($obj);

?>

