#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

// Listen on default port 5555, the webserver on the Yún
// will forward there all the HTTP requests for us.
YunServer server;

void setup() {
  // Bridge startup
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
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

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "buzz" command?
  if (command == "buzz") {
    buzzCommand(client);
  }
}

void buzzCommand(YunClient client) {
  int trigger;
  
  // Read whether to trigger or not
  // only accepts 1; any other value doesn't do anything
  trigger = client.parseInt();
  
  if (trigger == 1) {
    //open the doorlatch
    digitalWrite(12, HIGH);
    //shine the onboard LED for shits and giggles
    digitalWrite(13, HIGH);
    /*
      originally here I had the following line
        client.println("Door latch open");
      but this isn't synchronous communication
      the Yun sends all of the data back to the client at once at the end of this function
      as such, it's not useful for realtime status and only for informing the end user of final info
    */
    //keep buzzing the latch open for 7s
    delay(7000);
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
    client.println("Success");
  }

}

