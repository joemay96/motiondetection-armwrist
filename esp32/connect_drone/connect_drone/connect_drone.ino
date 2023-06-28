/* 
  WiFi code für den ESP32 um sich mit der Drohne zu Verbinden und Befehle an die Drohne zu senden 
  https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/
*/

#include "WiFi.h"

const char* ssid = "Potensic P7_687735";
const char* password = "";

const String[12] CDMS = {
  // 01100011 01100011 00000001 00000000 00000000 00000000 00000000, cc, RIGHT 80+ | LEFT 80-, FWD 80+ | BACK 80-, UP 80+ | DOWN 80-, Turn Right 80+ | T LEFT 80-, ??, 33
  // INIT
  "01100011 01100011 00000001 00000000 00000000 00000000 00000000",
  // IDLE
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000000 00000000 00110011",
  // START
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000001 00000001 00110011",
  // LAND
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000010 00000010 00110011",
  // FWD
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10001101 10011101 10000000 10000000 00000000 00010000 00110011",
  // BWD
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000001 01011001 10000000 10000000 00000000 11011000 00110011",
  // RIGHT
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10100111 01111011 10000000 10000000 00000000 11011100 00110011",
  // LEFT
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 01011001 01111010 10000000 10000000 00000000 00100011 00110011",
  // UP
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 11111110 10000000 00000000 01111110 00110011",
  // DOWN
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 00000001 10000000 00000000 10000001 00110011",
  // TURN_R
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 01101001 11111011 00000000 10010010 00110011",
  // TURN_L
  "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 01100101 00000101 00000000 01100000 00110011",
}

// IPAddress host(192, 168, 0, 1);
const char * host = "192.168.0.1";
const uint16_t port = 40000;

/* Config for sending */
const float POLLING_RATE = 1.0 / 15;

bool fresh_connect = true;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  check_and_connect();
}

void loop() {
  WiFiClient client;
  print_wifi_status();
  // check if the client is connected and if not try to reconnect the client
  if(!client.connected()) {
    if(!connect_to_drone()) {
      delay(500);
      return;
    }
  }
  // client is conntected to drone
  
  // if the connection is fresh - send init command
  if(fresh_connect) {
    client.write();
    // client.print();
    // CDMS[0]
  }
  

  // client.stop();
  delay(5000);
}

// Check the current connection and connect to the drone
void check_and_connect() {
  Serial.print("Connecting to Potensic drohne...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to Potensic drohne");
}

// print wifi status
void print_wifi_status() {
  Serial.print("Try to connect to: ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);
}

// connect the esp to the drone
bool connect_to_drone() {
  client.connect(host, port);
  if (!client.connected()) {
    Serial.println("Connection to drone failed!");
    return false;
  }
  fresh_connect = true;
  return true;
}


