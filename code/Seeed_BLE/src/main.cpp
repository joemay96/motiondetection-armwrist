/* Code for Seeed BLE Server which sends information to the clients.
 * Therefore every device which supports BLE can be connected to the Seeed microcontroller and read out the IMU data that gets boardcasted.
 */
#define ARDUINO_SEEED_XIAO_NRF52840_SENSE
#include <Arduino.h>
#include <ArduinoBLE.h>
#include "cmds.h"

// Weiß nicht ob die UUID hier random ist oder nicht
BLEService imuService(AW_BLE_SERVICE_ID); // Bluetooth® Low Energy LED Service
// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
// 20 indicates the maximum length of data you can send.
BLEStringCharacteristic imuCharacteristic(AW_BLE_SERVICE_CHARACTERISTIC, BLERead | BLENotify, 20);

int last_val = 0;

void initBLE()
{
  // begin initialization
  if (!BLE.begin())
  {
    Serial.println("starting Seeed BLE failed!");
    while (1)
      ;
  }

  // set advertised local name and service UUID:
  BLE.setLocalName(bleServerName);
  BLE.setAdvertisedService(imuService);
  // add the characteristic to the service
  imuService.addCharacteristic(imuCharacteristic);
  // add service
  BLE.addService(imuService);

  // start advertising
  BLE.advertise();

  Serial.println("Seeed BLE ready");
}

void setup()
{
  Serial.begin(9600);
  initBLE();
}

void loop()
{
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice client = BLE.central();
  Serial.println("Connecting...");
  delay(500);

  // if a device is connected:
  if (client)
  {
    Serial.print("Connected to device: ");
    // print the device's MAC address:
    Serial.println(client.address());

    // while the device is connected send IMU data
    while (client.connected())
    {
      //! Here the Motion Recognition later takes place and the enum value will be send to the other device
      //! Check if a new value was recognized
      // new_val != last_val
      // instead of if(true)...
      if (true)
      { // check for new data
        imuCharacteristic.setValue(CMD[5]);
      }
    }

    // when the central disconnects, print it out:
    Serial.print("Device disconnected: ");
    Serial.println(client.address());
  }
}