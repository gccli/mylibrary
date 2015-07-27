<?php
$a = [];
$keys = array('a', 'b', 'c', 'd', 'e', 'f', 'g');
$vals = array('i', 'j', 'k', 'x', 'y', 'z', 0x67);
$ai = 0x61;
foreach(range('a', 'z') as $c) {
    $a[$c] = $ai++;
}

print_r(array_change_key_case($a, CASE_UPPER));
print_r(array_chunk($a, 10, true));
$x = array_combine($keys, $vals);
print_r($x);
print_r(array_count_values($a));
print_r(array_diff_assoc($a, $x));
print_r(array_diff_key($a, $x));
print_r(array_diff($a, $x));
print_r(array_rand($a, 4));

?>
