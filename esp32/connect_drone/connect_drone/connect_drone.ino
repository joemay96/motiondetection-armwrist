/* 
  WiFi code für den ESP32 um sich mit der Drohne zu Verbinden und Befehle an die Drohne zu senden 
  https://techtutorialsx.com/2018/05/17/esp32-arduino-sending-data-with-socket-client/
*/

#include "WiFi.h"

const char* ssid = "Potensic P7_687735";
const char* password = "";

const String CMDS[] = {
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
};

// define how long every command should take - only IDLE, START and LAND need other time measures
// TODO: after the start and stop command their should come at least 1 bzw. 3 seconds idle signal before the connection is closed, so that the drone has enough time to stop.
const int CMD_LENGTH[] = {
  0, // INIT = 0
  1, // IDLE = 1
  2, // START = 2
  4, // LAND = 3
  1, // FWD = 4
  1, // BWD = 5
  1, // RIGHT = 6
  1, // LEFT = 7
  1, // UP = 8
  1, // DOWN = 9
  1, // TURN_R = 10
  1, // TURN_L = 11
};

// IPAddress host(192, 168, 0, 1);
const char *host = "192.168.0.1";
const uint16_t port = 40000;

/* Config for sending */
const float POLLING_RATE = 1.0 / 15;

bool fresh_connect = true;
unsigned long launch = 0;
unsigned long last_cmd = 0;

int test_series[] = {
  2,
  1,
  4,
  10,
  10,
  10,
  6,
  7,
  11,
  11,
  11,
  9,
  8,
  3,
  1,
  1,
  1
};

int test_series_length = 17;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Potensic drohne");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(WiFi.status());
  }
  Serial.print("Connected to Potensic drohne with IP: ");
  Serial.println(WiFi.localIP());
}

void loop() 
{

/*
 * TODO: 
 * - check for BLE connection to wristband
 * - check then what command is coming and with that send the correct command to the drone
 */
  // TODO: getting the IMU signals via bluetooth and one is client.stop();

  drone();
}

// Check the current connection and connect to the drone
// void check_and_connect() {
//   Serial.print("Connecting to Potensic drohne...");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.print("Connected to Potensic drohne");
// }

// print wifi status
void print_wifi_status() {
  Serial.print("Try to connect to: ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);
}

// connect the esp to the drone
bool connect_to_drone(WiFiClient client) {
  client.connect(host, port);
  if (!client.connected()) {
    Serial.println(WiFi.status());
    Serial.println("Connection to drone failed!");
    return false;
  }
  fresh_connect = true;
  launch = millis();
  last_cmd = millis();
  return true;
}

// TODO: Check if i want to do send_command like this

// send command and everything that is included with it
void send_command(WiFiClient client, String cmd) {
  last_cmd = millis();
  client.print(cmd);
}
// send command and everything that is included with it
void send_command_with_number(WiFiClient client, int cmd_number) {
  last_cmd = millis();
  client.print(CMDS[cmd_number]);
}

// send a series of commands to the drone
void send_test_series(WiFiClient client) {
  int i = 0;
  last_cmd = millis();
  long last_frame = millis();

  while(i < test_series_length) {
    String cmd = CMDS[i];
    int cmd_length = CMD_LENGTH[test_series[i]];
    long elapsed_time = millis() - last_cmd;

    // current command is over - go to next command
    if(elapsed_time > cmd_length*1000000) {
      i++;
      last_cmd = millis();
      continue;
    }
    // send command 
    else{
      client.print(cmd);
    }

    // Handle sleeping in length of the polling rate
    long since_last_frame = millis() - last_frame;
    if (since_last_frame < POLLING_RATE)
    {
      delay(POLLING_RATE - since_last_frame);
    }
    last_frame = millis();
  }
}

// the main method for the drone stuff
// TODO: command parameter coming from the IMU and so on
void drone()
{
  // check if the client is connected and if not try to reconnect the client
  WiFiClient client;
  client.connect(host, port);
  // if(!client.connected()) {
  //   int stat = client.connect(host, port);
  //   Serial.println(stat);
  //   if (!client.connected()) {
  //     Serial.println(WiFi.status());
  //     Serial.println("Connection to drone failed!");
  //     delay(1000);
  //     return;
  //   }
    fresh_connect = true;
    launch = millis();
    last_cmd = millis();
  // }
  // check if the client is connected and if not try to reconnect the client
  /* if(!client.connected()) {
    connect_to_drone(client);
    if(!connect_to_drone(client)) {
      delay(500);
      return;
    }
    print_wifi_status();
  } */
  // client is conntected to drone
  
  // if the connection is fresh - send init command
  if(fresh_connect) {
    // sending init command for communication start - no time required as the command only gets send once
    client.print(CMDS[0]);
  }

  // TODO: sort this out for IMU messages
  // send series of commands to drone
  send_test_series(client);
}


