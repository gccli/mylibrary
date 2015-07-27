<?php

echo "Use mysql\n";
function connect_mysql($host = "172.16.1.1",$user = "root",$password = "my-mysqlpw",$dbname = "world")
{
  $mysqli = new mysqli($host, $user, $password, $dbname);
  if ($mysqli->connect_errno) {
    printf("Connect failed: %s\n", $mysqli->connect_error);
    return -1;
  }

  $query = "SELECT Name, CountryCode FROM City ORDER by ID DESC LIMIT 50,5";
  if ($result = $mysqli->query($query)) {
    while ($row = $result->fetch_assoc()) {
      printf ("%s (%s)\n", $row["Name"], $row["CountryCode"]);
    }
  }
  $mysqli->close();

  return 0;
}

connect_mysql();

?>
