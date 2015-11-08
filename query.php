<?php
ini_set('display_errors',1);  error_reporting(E_ALL);
if (isset($_GET['id'])) {
    $servername = "localhost";
    $username = "root";
    $password = "asdfasdf";

    $conn = mysqli_connect($servername,$username,$password,"flashcards");
    if (!$conn) die ("Connection failed: " . mysqli_connect_error());
 
    $sql = "SELECT Word, Definition, Activation from studies where UID='{$_GET['id']}'";
    $result = $conn->query($sql);
    $words = array();
    $min_activation = 10000;
    $priority_word = 0;
    $def = 0;
    if ($result->num_rows > 0) {
        while ($row = $result->fetch_assoc()) {
 	    $words[$row['Word']]["Definition"] = $row['Definition'];
            $words[$row['Word']]["Activation"] = $row['Activation'];
        }
    }
    $conn->close();
	$priority_word = array_rand($words);
    print json_encode(array("word" => $priority_word, "definition" => $words[$priority_word]["Definition"]));
    
    /*$words = array(
        "Ubiquitous" => "Everywhere, occuring in every possible setting.",
        "Funny" => "Evoking humor, causing laughter",
        "Jester" => "A figure in mideval(sp) courts who entertained people",
        "Kazoo" => "An instrument; one from Toys-R-Us was used to produce the hit pop song Talk Dirty to Me");
	
    $rand_word = array_rand($words);
    echo '{"word":"';
    echo $rand_word;
    echo '","definition":"';
    echo $words[$rand_word];
    echo '"}';*/
}

?>
