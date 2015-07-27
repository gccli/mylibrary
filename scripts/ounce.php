<?php

$ounce = 31.103481;

function get_usdcny()
{
    $base = 'http://download.finance.yahoo.com/d/quotes.csv?';
    $query = 's=USDCNY=X&f=nab';
    $url = $base . $query;
    $content = file_get_contents($url);
    print $url.PHP_EOL;
    print $content.PHP_EOL;

    return (float) split(',', $content)[2];
}

$average = 1200;
$range = 10;
if ($argc > 1) $average = $argv[1];
if ($argc > 2) $range = $argv[2];

$usdcny = get_usdcny();
foreach(range($average-$range, $average+$range) as $i) {
    if (abs($average-$i) <= 2) 
        printf("\033[32m%d - %.3f\033[0m\n", $i, $i*$usdcny/$ounce);
    else 
        printf("%d - %.3f\n", $i, $i*$usdcny/$ounce);
}

?>