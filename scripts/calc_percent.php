<?php
array_shift ($argv);

$sum = 0.0;
foreach($argv as $a) {
    $sum += $a;
}

foreach($argv as $a) {
    printf("%f\n",  round ($a/$sum*100.0, 2));
}

?>