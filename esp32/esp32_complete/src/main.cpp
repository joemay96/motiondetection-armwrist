/*
  author: @joemay96
  Code for the ESP32 that is needed to connect to the motion detection handwrist via BLE and sending the commands to the quadrocopter.
*/

#include <Arduino.h>
#include "BLEDevice.h"
#include <Wire.h>
#include "BLE.h"

// BLE Server name (the other ESP32 name running the server sketch)

/* UUID's of the service, characteristic that we want to read*/
static BLEUUID imuServiceUUID(BLE_SERVICE_ID);
static BLEUUID imuCharacteristicUUID(BLE_SERVICE_CHARACTERISTIC);

// Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

// Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

// Characteristicd that we want to read
static BLERemoteCharacteristic *imuCharacteristic;

// TODO: kann vielleicht raus Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

// Variables to store imu values
int imuData;
boolean newData = false;

// This is an important function: When the BLE Server sends a new data react with the notify property
static void imuNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  imuData = (int)*pData;
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

  //! not sure if this works
  // if(imuCharacteristicUUID.toString() == "<NULL>")
  // könnte funktionieren, falls das unten der Fehler ist
  // if (imuCharacteristicUUID == nullptr)
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
    Serial.printf("%s \n", advertisedDevice.getServiceUUID().toString());
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
  pBLEScan->start(59);
  //* falls das obere nicht klappt
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  Serial.println("BLE Setup finished");
}

void setup()
{
  // Start serial communication
  Serial.begin(9600);
  Serial.println("Start ESP32 BLE");

  // Starting the whole BLE Connection process
  startBLEConnection();
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
    if (connectToServer(*pServerAddress))
    {
      Serial.println("Connected to IMU Wristband");
      connected = true;
    }
    else
    {
      // TODO: change to run the setup code again
      Serial.println("Failed to connect to the server; Restart your device.");
    }
    doConnect = false;
  }
  // we are connected and a new CMD got received
  if (newData)
  {
    newData = false;
    // TODO: send the data as a CMD to the quadrocopter
    Serial.println(imuData);
  }
}
