<?php
echo "-------- boolean type\n";

var_dump((bool) "");
var_dump((bool) 0);
var_dump((bool) "0");
var_dump((bool) array());
var_dump($a);

var_dump(True == true);
var_dump(0 == 'all'); // TRUE, take care
var_dump(0 === 'all');

?>

<?php
echo "-------- integer type\n";
$a = 0b11111111;
var_dump($a);

var_dump(01090); // 010 octal = 8 decimal, he rest of the number is ignored
var_dump(0x100f);
?>

<?php
echo "-------- floating point type\n";

// NaN
$nan = acos(8);
var_dump($nan, is_nan($nan));

$a = 1.234;
$b = 1.2e3;
$c = 7E-10;
$a = 1.23456789;
$b = 1.23456780;
$epsilon = 0.00001;

if(abs($a-$b) < $epsilon) {
    echo "true";
}
?>

<?php
echo "-------- string\n";
$string = <<< EOD
single quoted
double quoted
heredoc syntax
nowdoc syntax
EOD;

// heredoc 
echo $string."\n";

class foo
{
    var $foo;
    var $bar;
    function foo() {
        $this->foo = 'Foo';
        $this->bar = array('Bar1', 'Bar2', 'Bar3');
    }
}

$foo = new foo();
$name = 'MyName';

// heredoc 
echo <<<EOT
My name is "$name". I am printing some $foo->foo.
    Now, I am printing some {$foo->bar[1]}.
This should print a capital 'A': \x41\n
EOT;

// nowdoc
$string = <<<'EOT'
My name is "$name". I am printing some $foo->foo.
    Now, I am printing some {$foo->bar[1]}.
This should not print a capital 'A': \x41
EOT;

// nowdoc 
echo $string.PHP_EOL;
echo PHP_EOL.PHP_EOL;

error_reporting(E_ALL);
$great = 'fantastic';
echo "This is { $great}".PHP_EOL;
echo "This is ${great}".PHP_EOL;
#echo "This square is {$square->width}00 centimeters broad.";
#echo "This works: {$arr['key']}";
#echo "This works: {$arr[4][3]}";
#echo "This is wrong: {$arr[foo][3]}";
#echo "This works: " . $arr['foo'][3];
#echo "This is the value of the var named by the return value of getName(): {${getName()}}";
class beers {
    const softdrink = 'rootbeer';
    public static $ale = 'ipa';
}
$rootbeer = 'A & W';
#$ipa = 'Alexander Keith\'s';
echo "I'd like an {${beers::softdrink}}\n";
echo "I'd like an {${beers::$ale}} !rubbish syntax!\n";

?>


<?php
echo "-------- array\n";
$array = array(
    1    => "a",
    "1"  => "b",
    1.5  => "c",
    true => "d",
);
var_dump($array);

$array = array(
    "foo" => "bar",
    42    => 24,
    "multi" => array(
        "dimensional" => array(
             "array" => "foo"
        )
    )
);
var_dump($array["foo"]);
var_dump($array[42]);
var_dump($array["multi"]["dimensional"]["array"]);


$array = array(1, 2, 3, 4, 5);
print_r($array);

foreach ($array as $i => $value) {
    unset($array[$i]);
}
print_r($array);

$array[] = 6;
print_r($array);

$array = array_values($array);
$array[] = 7;
print_r($array);
?>


<?php 
echo "-------- object\n";
class fooo
{
    function do_foo(){
        echo "Doing foo.".PHP_EOL; 
    }
}

$bar = new fooo;
$bar->do_foo();

$obj = (object) 'ciao';
echo $obj->scalar.PHP_EOL;  // outputs 'ciao'
?>

<?php 
echo "-------- resource\n";

$fp = fopen("foo","w");
echo get_resource_type($fp)."\n";
$fp = gzopen("/tmp/types.php.gz", "r");
echo get_resource_type($fp)."\n";
$content=gzread($fp, 100);
gzclose ($fp);
?>