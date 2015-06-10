#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

// Listen on default port 5555, the webserver on the Yún
// will forward there all the HTTP requests for us.
YunServer server;

void setup() {
  // Bridge startup
  // Connect the switch lead to pin 12 for control
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  // pin 13 is the onboard LED, for feedback
  // you can also control the pin itself, but it doubles up with the LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);
    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

//set up a way to process URL's

//with the below we want the URL to be
//http://doorlatch.local/arduino/buzz/{seconds_to_hold_open}
//so we set up "buzz" as the handler
void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "buzz" command?
  if (command == "buzz") {
    buzzCommand(client);
  }
}

//takes one mandatory argument of the number of seconds to hold the door open
//valid values are between 1 and 10 seconds
void buzzCommand(YunClient client) {
  int seconds_to_hold_open;
  String jsonp_return;
  
  // Read seconds_to_hold_open
  seconds_to_hold_open = client.parseInt();
  
  //check the length of the hold
  //valid lengths are between 1 and 10s
  //if it is longer than 10s, set to 10s
  if (seconds_to_hold_open > 10) {
     seconds_to_hold_open = 10;
  }
  
  //if it is negative set it to 1
  if (seconds_to_hold_open < 0) {
     seconds_to_hold_open = 1;
  }
  
  //open the doorlatch
  digitalWrite(12, HIGH);
  //shine the onboard LED for shits and giggles
  digitalWrite(13, HIGH);
  
  //keep buzzing the latch open for seconds_to_hold_open
  //delay() takes its argument in ms so we x1000
  delay(seconds_to_hold_open * 1000);
  
  /*
    note that the default web server timeout on the Yun is 5 seconds
    the arduino will faithfully execute this code even with the timeout
    but the Lua webserver will return a 500 error to the requesting web client if it times out
    which will probably screw your web client code up if it is properly checking for HTTP status
    you thus need to make sure your socket_timeout in /etc/config/arduino is greater than the delay above
  */
  //close the doorlatch
  digitalWrite(12, LOW);
  //turn off the LED
  digitalWrite(13, LOW);
  //send the client final status message
  //construct string first since println doesn't like concatenation
  //and compiler does weird things
  //sending it as JSONP so CORS issues don't come up
  jsonp_return = "seconds_held_open({\"value\": ";
  jsonp_return += seconds_to_hold_open;
  jsonp_return += "});";
  client.println(jsonp_return);
}

