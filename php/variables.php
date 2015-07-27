<?php
namespace mycode;

// Predefined Variables
echo $_SERVER['HOME'].PHP_EOL;
#print_r($GLOBALS);
#print_r($_SERVER);

// Constants 
define("CONSTANT", "Hello world.");
echo CONSTANT.PHP_EOL;
const CONSTANTI = 'Helolo World const';
echo \mycode\CONSTANTI.PHP_EOL;

#print_r(get_defined_constants());

// Magic constants
// __LINE__ The current line number of the file.
// __FILE__ The full path and filename of the file with symlinks resolved. 
// __DIR__ The directory of the file.
// __FUNCTION__The function name.
// __CLASS__The class name. The class name includes the namespace it was declared in (e.g. Foo\Bar).
// __TRAIT__The trait name. The trait name includes the namespace it was declared in (e.g. Foo\Bar).
// __METHOD__The class method name.
// __NAMESPACE__The name of the current namespace.
printf("%s:%s:%s:%s:%s:%s:%s:%s\n", __DIR__,__FILE__,__LINE__,__FUNCTION__,__CLASS__,__TRAIT__,__METHOD__,__NAMESPACE__);

?>