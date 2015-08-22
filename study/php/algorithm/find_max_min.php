<?php

function find_max_min($inputs)
{
    $min = $inputs[0];
    $max = $inputs[0];
    $n = count($inputs);
    $i = ($n % 2 != 0) ? 1: 0;
    for(; $i < count($inputs); $i += 2) {
        if ($inputs[$i] > $inputs[$i+1]) {
            if ($inputs[$i] > $max) $max = $inputs[$i];
            if ($inputs[$i+1] < $min) $min = $inputs[$i+1];
        } else if ($inputs[$i] < $inputs[$i+1]){
            if ($inputs[$i+1] > $max) $max = $inputs[$i+1];
            if ($inputs[$i] < $min) $min = $inputs[$i];
        }
    }

    return array($max, $min);
}

$n = 20;
if ($argc > 1) $n = (int)$argv[1];
$inputs = range(0, $n);
$inputs[] = mt_rand(0, $n);
$inputs[] = mt_rand(0, $n);
shuffle($inputs);
print implode(" ", $inputs).PHP_EOL;
print implode(" ", find_max_min($inputs)).PHP_EOL;

?>