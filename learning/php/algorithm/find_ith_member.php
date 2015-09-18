<?php

function swap(&$x, &$y) {
    $tmp = $x;
    $x = $y;
    $y = $tmp;
}

function partition(array &$inputs, $p, $r) {

    // randomized pivot
    $i = mt_rand($p, $r);
    swap($inputs[$i], $inputs[$r]);

    $v = $inputs[$r];
    $j = $p - 1;
    for($i=$p; $i<$r; $i++) {
        if ($inputs[$i] <= $v) {
            $j++;
            swap($inputs[$i], $inputs[$j]);
        }
    }

    swap($inputs[$r], $inputs[$j+1]);
    return $j+1;
}

/**
 * returns the i th smallest element of the array $inputs[p..r]
 */
function randomized_select(array &$inputs, $p, $r, $i) {
    if ($p == $r) {
        return $inputs[$p];
    }

    $q = partition($inputs, $p, $r);

    $k = $q - $p + 1;
    if ($i == $k) return $inputs[$q];
    else if ($i < $k) {
        return randomized_select($inputs, $p, $q-1, $i);
    } else {
        return randomized_select($inputs, $q+1, $r, $i-$k);
    }
}

$n = 20;
if ($argc > 1) $n = (int)$argv[1];
$inputs = range(1, $n);
shuffle($inputs);
printf("array(%d):\n%s\n", count($inputs), implode(" ", $inputs));

$i = mt_rand(1, $n);
printf("find %d'th smallest of array: ", $i);
print randomized_select($inputs, 0, $n-1, $i).PHP_EOL;

?>