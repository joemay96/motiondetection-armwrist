/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */

#include "main.h"
// WiFi network name and password:
// const char *networkName = "Potensic P7_Laptop";
const char* networkName = "Potensic P7_687735";

// IP address to send UDP data to:
//  either use the ip address of the server or
//  a network broadcast address
const char *udpAddress = "192.168.0.1";
const int udpPort = 40000;

// Are we currently connected?
boolean connected = false;

// The udp library class
WiFiUDP udp;

void setup()
{
  // Initialize hardware serial:
  Serial.begin(115200);

  // Connect to the WiFi network
  connectToWiFi(networkName);
}

void loop()
{
  bool init = false;
  // only send data when connected
  if (connected)
  {
    if (!init) {
      sendCMD(CMD::INIT);
      init = true;
    }
    sendCMD(CMD::START);
  }
  delay(POLLING_RATE);
}

void sendCMD(CMD cmd)
{
  // Send a packet
  udp.beginPacket(udpAddress, udpPort);
  char *data = (char *)CMD_LIST[cmd].data();
  int len = CMD_LIST[cmd].size();
  for (int i = 0; i < len; i++)
  {
    printf("%02x", data[i]);
    udp.print(data[i]);
  }
  printf("\n");
  udp.endPacket();
}

void connectToWiFi(const char *ssid)
{
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  // register event handler
  WiFi.onEvent(WiFiEvent);

  // Initiate connection
  WiFi.begin(ssid);

  Serial.println("Waiting for WIFI connection...");
}

// wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // When connected set
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
    // initializes the UDP state
    // This initializes the transfer buffer
    udp.begin(WiFi.localIP(), udpPort);
    connected = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    connected = false;
    break;
  default:
    break;
  }
}