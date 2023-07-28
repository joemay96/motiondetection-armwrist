/* 
  WiFi code für den ESP32 um sich mit der Drohne zu Verbinden und Befehle an die Drohne zu senden 
  https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/
*/

#include "WiFi.h"

const char* ssid = "iPhone von Josef";
const char* password = "pw1234567890";

const uint16_t port = 8090;
const char * host = "172.20.10.6";
// IPAddress host(10,104,75,203);

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client;

  Serial.print("Try to connect to: ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  client.connect(host, port);
  
  Serial.println(client.connected());

  if (!client.connected()) { //!client.connect(host, port)
    Serial.println("Connection to host failed");
    delay(1000);
    return;
  }

  Serial.println("Connection successful!");
  client.print("Hello from ESP32!");
  Serial.println("Disconnecting...");
  client.stop();
  delay(5000);
}

