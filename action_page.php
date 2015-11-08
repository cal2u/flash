<?php
$servername = "localhost";
$username = "root";
$password = "asdfasdf";
$dbname = "flashcards";
// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 
for ($i = 1; $i <= 5; $i+=1) { 
if (isset($_POST["id"]) && isset($_POST["term{$i}"]) && isset($_POST["def{$i}"]) && $_POST["term{$i}"] != "" && $_POST["def{$i}"] != "" ) {
      $sql = "INSERT INTO studies (UID, Word, Definition, Activation, Permanence) VALUES ('{$_POST["id"]}', '{$_POST["term{$i}"]}', '{$_POST["def{$i}"]}', '10', '10')";
      $conn->query($sql);
   }
}

$conn->close();
echo "Thanks for making more kitties happy!!!!!</br>";
include("index.html");
?>
