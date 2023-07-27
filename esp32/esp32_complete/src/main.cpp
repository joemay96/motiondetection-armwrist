/*
  author: @joemay96
  Code for the ESP32 that is needed to connect to the motion detection handwrist via BLE and sending the commands to the quadrocopter.
*/

#include "main.h"

/*
  BLE Setup
*/

/* UUID's of the service, characteristic that we want to read*/
static BLEUUID imuServiceUUID(BLE_SERVICE_ID);
static BLEUUID imuCharacteristicUUID(BLE_SERVICE_CHARACTERISTIC);

// Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
// TODO: kann vielleicht raus?
static boolean bleConnected = false;

// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

// Characteristicd that we want to read
static BLERemoteCharacteristic *imuCharacteristic;

// Variables to store imu values
int imuData;
boolean newData = false;

/*
  WiFi Setup
*/

WiFiUDP udp;

// WiFi network name and password:
const char *networkName = "Potensic P7_687735";

// IP address to send UDP data to: either use the ip address of the server or a network broadcast address
const char *udpAddress = "192.168.0.1";
const int udpPort = 40000;

// WiFi currently connected?
boolean wifiConnected = false;
boolean droneInit = false;

/*
  BLE Methods
*/

// This is an important function: When the BLE Server sends a new data react with the notify property
static void imuNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  imuData = (int)*pData;
  Serial.println("Befehl gesendet");
  Serial.println(imuData);
  newData = true;
}

// Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress)
{
  BLEClient *pClient = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println("Connected to Seeed");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(imuServiceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find Service UUID: ");
    Serial.println(imuServiceUUID.toString().c_str());
    return false;
  }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  imuCharacteristic = pRemoteService->getCharacteristic(imuCharacteristicUUID);

  if (imuCharacteristicUUID.toString() == "<NULL>")
  {
    Serial.print("Failed to find characteristic UUID");
    return false;
  }

  Serial.println("Found IMU characteristic");

  // Assign callback functions for the Characteristics
  imuCharacteristic->registerForNotify(imuNotifyCallback);
  return true;
}

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.printf("%s \n", advertisedDevice.toString().c_str());
    // Serial.printf("%s \n", advertisedDevice.getServiceUUID().toString());
    if (advertisedDevice.getServiceUUID().toString() == imuServiceUUID.toString())
    {                                                                 // Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop();                             // Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
      doConnect = true;                                               // Set indicator, stating that we are ready to connect
      Serial.println("IMU Wristband found. Connecting...");
    }
  }
};

void startBLEConnection()
{
  // Init BLE device
  BLEDevice::init("ESP Drone Connector");

  // Retrieve a Scanner and set the callback we want to use to be informed when we have detected a new device. Specify that we want active scanning and start the scan to run for 30 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
  //* falls das obere nicht klappt
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  Serial.println("BLE Setup finished");
}

/*
  WiFi Methods
*/

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
    wifiConnected = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    wifiConnected = false;
    break;
  default:
    break;
  }
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

unsigned long long time_stamp = 0;

void setup()
{
  // Start serial communication
  Serial.begin(115200);
  Serial.println("Start ESP32 BLE");

  // Starting the whole BLE Connection process
  // startBLEConnection();
  connectToWiFi(networkName);

  Serial.println("Start WiFi connection to drone");
}

void loop()
{
  /*
    React to new incomming commands send by the BLE Connection
  */
  // If the flag "doConnect" is true then we have scanned for and found the desired BLE Server with which we wish to connect.
  // Now we connect to it. Once we are connected we set the connected flag to be true.
  if (doConnect)
  {
    // pServerAdress gets populated in setup()
    // if (connectToServer(*pServerAddress))
    // {
    //   Serial.println("Connected to IMU Wristband");
    //   bleConnected = true;
    // }
    // else
    // {
    //   // TODO: change to run the setup code again
    //   Serial.println("Failed to connect to the server; Restart your device.");
    // }
    doConnect = false;
  }
  // we are connected and a new CMD got received
  // if (true) //! newData
  // {
  //! newData = false;
  // only send data to the drone when connected
  if (wifiConnected)
  {
    if (!droneInit)
    {
      time_stamp = millis();
      // send init command to the drone
      sendCMD(CMD::INIT);
      droneInit = true;
    }
    // CMD recivedCMD = CMD(imuData);
    //! sendCMD(CMD(recivedCMD));
    if (millis() - time_stamp <= 4000)
    {
      sendCMD(CMD::START);
    }
    else if (millis() - time_stamp <= 6000)
    {
      sendCMD(CMD::TURN_L);
    }
    else if (millis() - time_stamp <= 8000)
    {
      sendCMD(CMD::TURN_R);
    }
    else if (millis() - time_stamp <= 9000)
    {
      sendCMD(CMD::BWD);
    }
    else if (millis() - time_stamp <= 10000)
    {
      sendCMD(CMD::FWD);
    }
    else if (millis() - time_stamp <= 12000)
    {
      sendCMD(CMD::LAND);
    }
    delay(POLLING_RATE);
  }
  // else
  // {
  // droneInit = false;
  // sendCMD(CMD::INIT);
  // }
  // }
  // sendCMD(CMD::INIT);
}
