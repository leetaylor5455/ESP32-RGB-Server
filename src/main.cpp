#include <Arduino.h>

#include <WiFi.h>

const char* ssid = "yourSSID";
const char* password = "yourPassword";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Set web server port number to 80
WiFiServer server(80);

// Decode HTTP GET value
String redString = "0";
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

int redVal = 0;
int greenVal = 255;
int blueVal = 72;
bool isTurnedOn = true;

// Variable to store the HTTP req  uest
String header;

const int redLED = 16;
const int greenLED = 17;
const int blueLED = 18;

const int indicatorLED = 19;

const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;

const int pwmFrequency = 5000;
const int pwmResolution = 8;

void setColour(int r, int g, int b, bool temp = false) {
  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);

  if (!temp) { // for turning off an on again so values are not set to 0
    redVal = r;
    greenVal = g;
    blueVal = b;
  }
  
  if (r + g + b <= 0) {
    isTurnedOn = false;
  } else {
    isTurnedOn = true;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }


  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  server.begin();

  ledcSetup(redChannel, pwmFrequency, pwmResolution);
  ledcSetup(greenChannel, pwmFrequency, pwmResolution);
  ledcSetup(blueChannel, pwmFrequency, pwmResolution);

  
  ledcAttachPin(redLED, redChannel);
  ledcAttachPin(greenLED, greenChannel);
  ledcAttachPin(blueLED, blueChannel);

  setColour(redVal, 0, greenVal);
}

void loop() {

  // setColour(255,255,255);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {            // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connection: close");
            client.println();

            // Request sample: /?r201g32b255&
            // Red = 201 | Green = 32 | Blue = 255
            if(header.indexOf("GET /?r") >= 0) {
              pos1 = header.indexOf('r');
              pos2 = header.indexOf('g');
              pos3 = header.indexOf('b');
              pos4 = header.indexOf('&');

              redString = header.substring(pos1+1, pos2);
              greenString = header.substring(pos2+1, pos3);
              blueString = header.substring(pos3+1, pos4);

              setColour(redString.toInt(), greenString.toInt(), blueString.toInt());
            }
            if(header.indexOf("GET /status") >= 0) {
              if (!isTurnedOn) {
                client.print("off");
                client.println();
              } else {
                Serial.println("Status requested.");
                client.print("r");
                client.print(redVal);
                client.print("g");
                client.print(greenVal);
                client.print("b");
                client.print(blueVal);
                client.println();
              }
            }
            if(header.indexOf("GET /off") >= 0) {
              setColour(0, 0, 0, true);
              client.print("of");
            } else if (header.indexOf("GET /on") >= 0) {
              setColour(redVal, greenVal, blueVal);
              client.print("on");
            }
            
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}