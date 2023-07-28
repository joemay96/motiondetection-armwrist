/* This script includes all the Code that is used for the BLE connection from the Seeed microcontroller to the ESP32 from the ESP32 side. */

/*
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

// Temperature Characteristic and Descriptor
#ifdef temperatureCelsius
  BLECharacteristic bmeTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));
#else
  BLECharacteristic bmeTemperatureFahrenheitCharacteristics("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor bmeTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2901));
#endif

// Humidity Characteristic and Descriptor
BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));
*/

#include "BLEDevice.h"
#include <Wire.h>

//BLE Server name (the other ESP32 name running the server sketch)
#define imuServerName "IMU_Handwrist"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID imuServiceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");

static BLEUUID imuCharacteristicUUID("19B10001-E8F2-537E-4F6C-D104768A1214");

/***********************************************************************/

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;
 
//Characteristicd that we want to read
static BLERemoteCharacteristic* imuCharacteristic;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

//Variables to store imu values
struct Imu_vals {
  int ax;
  int ay;
  int az;
  int gx;
  int gy;
  int gz;
}

// Imu_vals imuData;
char* imuData;

//Flags to check whether new imu values are available
boolean newImu = false;

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println("Connected to BLE server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(imuServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find imu service UUID: ");
    Serial.println(imuServiceUUID.toString().c_str());
    return false;
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  imuCharacteristic = pRemoteService->getCharacteristic(imuCharacteristicUUID);

  if (imuCharacteristicUUID == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }

  Serial.println("Found IMU characteristic");
 
  //Assign callback functions for the Characteristics
  imuCharacteristic->registerForNotify(imuNotifyCallback);
  return true;
}

//When the BLE Server sends a new imu reading with the notify property
static void imuNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  // imuData = pData;
  imuData = (char *) pData;
  newImu = true;
  Serial.print(pData);
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == imuServiceUUID) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("IMU Wristband found. Connecting...");
    }
  }
};


void setup() {
  //Start serial communication
  Serial.begin(115200);
  Serial.println("Start ESP32");

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we have detected a new device. Specify that we want active scanning and start the scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired BLE Server with which we wish to connect. Now we connect to it. Once we are connected we set the connected flag to be true.
  if (doConnect == true) {
    // pServerAdress gets populated in setup()
    if (connectToServer(*pServerAddress)) {
      Serial.println("Connected to IMU Wristband.");
      //Activate the Notify property of each Characteristic
      imuCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902)sssss)->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
    } else {
      // TODO: change to run the setup code again
      Serial.println("Failed to connect to the server; Restart your device.");
    }
    doConnect = false;
  }
  //if new temperature readings are available, print in the OLED
  if (newImu){
    newImu = false;
    printImuValeus();
  }
  delay(1000); // Delay a second between loops.
}


void printImuValues() {
  Serial.print();
}
