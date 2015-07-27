<?php

function time_slot($time, $mode)
{
    list($h,$m,$s) = split(':', $time);
    $slot = 0;
    if ($mode == 'floor') {
        $slot = floor($m/10);
    } else if ($mode == 'ceil') {
        $slot = ceil($m/10);
    } else {
        $slot = round($m/10);
    }
    
    return (int)(6*($h-1) + $slot);
}

function time_slot_range($start, $end)
{
    return range(time_slot($start, 'floor'), time_slot($end, 'ceil'));
}

print_r(time_slot_range($argv[1], $argv[2]));

?>