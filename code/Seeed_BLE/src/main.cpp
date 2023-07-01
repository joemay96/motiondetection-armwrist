/* Code for Seeed BLE Server which sends information to the clients.
 * Therefore every device which supports BLE can be connected to the Seeed microcontroller and read out the IMU data that gets boardcasted.
 */

#include <Arduino.h>
#include <ArduinoBLE.h>

// Weiß nicht ob die UUID hier random ist oder nicht
BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service
// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
// 20 indicates the maximum length of data you can send.
BLEStringCharacteristic imuCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

#define bleServerName "IMU_Handwrist"

int last_val = 0;

void setup()
{
  Serial.begin(9600);

  // begin initialization
  if (!BLE.begin())
  {
    Serial.println("starting Seeed BLE failed!");
    while (1)
      ;
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("IMU_Handwrist");
  BLE.setAdvertisedService(imuService);

  // add the characteristic to the service
  imuService.addCharacteristic(imuCharacteristic);
  // add service
  BLE.addService(imuService);

  // start advertising
  BLE.advertise();

  Serial.println("Seeed BLE ready");
}

void loop()
{
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a device is connected:
  if (central)
  {
    Serial.print("Connected to device: ");
    // print the device's MAC address:
    Serial.println(central.address());

    // while the device is connected send IMU data
    while (central.connected())
    {
      // TODO: get IMU data
      if (true)
      { // check for new data
        imuCharacteristic.value("Hello world!");
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Device disconnected: "));
    Serial.println(central.address());
  }
}