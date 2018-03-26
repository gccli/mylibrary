<?php

function add_month($t, $offset) {
    $days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

    $y = intval(date('Y', $t));
    $m = intval(date('m', $t));
    $d = intval(date('d', $t));

    $y = $y + floor(($m+$offset-1)/12);
    if (($y % 4) == 0) $days[1] = 29;

    $m = ($m+$offset%12-1)%12+1;
    $d = min($d, $days[$m-1]);
    $newtime =  sprintf("%04d-%02d-%02d %s", $y, $m, $d, date('H:i:s', $t));

    return $newtime;
}

date_default_timezone_set('Asia/Shanghai');
$t = strtotime("2017-03-31 09:19:33");
$lastt = $t;
$loop=600;
for ($o=1; $o <= $loop; $o++) {
    $dt = mon_offset($t, $o);
    $ts = strtotime($dt);

    $days = ($ts-$lastt)/86400;
    $lastt=$ts;
    echo date('Y-m-d H:i:s', $t) . " + $o month = $dt offset=($days)\n";
}

?>
