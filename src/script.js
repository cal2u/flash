var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var user_id = "";

function sendResult(result, wid) {
  // We will request the word here
  var url = 'http://flashcardkitties.com/query.php?id='+user_id+'&wid='+wid+'&r='+result;
  console.log(url);
  xhrRequest(url, 'GET', 
    function(responseText) {
      console.log("Sent result: "+result);
    }
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

function getWord() {
  if (user_id === "") {
        var dictionary = {
        'KEY_RESEND_REQUEST': 1
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Resend request sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending user info info to Pebble!');
        }
      );
  }
  // We will request the word here
  var url = 'http://flashcardkitties.com/query.php?id='+user_id;
  
  xhrRequest(url, 'GET', 
    function(responseText) {
      console.log(responseText);
      // responseText contains a word and definition
      var json = JSON.parse(responseText);

      // We have a word
      var word = json.word;
      //console.log("word is: "+word);

      // And a definition
      var definition = json.definition;
      //console.log("definition is: "+definition);
      var wid = json.wid;
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_WORD': word,
        'KEY_DEFINITION': definition,
        'KEY_WORD_ID': wid
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Word info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending word info to Pebble!');
        }
      );
    }
  );
}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    var json = e.payload;
    if (json.hasOwnProperty("KEY_REQUEST_WORD")) {
        // update
        console.log("update requested");
        getWord(); 
    }
    else if (json.hasOwnProperty("KEY_RESPONSE")) {
      sendResult(json.KEY_RESPONSE, json.KEY_WORD_ID);
    }
    else if (json.hasOwnProperty("KEY_USER_ID")) {
      user_id = json.KEY_USER_ID;
      console.log("uid set to "+user_id);
    }
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://flashcardkitties.com/config-page.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Configuration window returned: ' + e.response);
  user_id = e.response;
  // Assemble dictionary using our keys
      var dictionary = {
        'KEY_USER_ID': user_id
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Message sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending user info info to Pebble!');
        }
      );
});
