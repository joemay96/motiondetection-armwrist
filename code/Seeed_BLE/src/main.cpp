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
BLEIntCharacteristic imuCharacteristic(AW_BLE_SERVICE_CHARACTERISTIC, BLERead | BLENotify);

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
  BLE.setDeviceName(bleServerName);
  BLE.setLocalName(bleServerName);
  BLE.setAdvertisedService(imuService);
  // add the characteristic to the service
  imuService.addCharacteristic(imuCharacteristic);

  //* sending an initial value?
  // imuCharacteristic.writeValue(0);

  // add service
  BLE.addService(imuService);

  // start advertising
  BLE.advertise();

  // print address
  Serial.print("Address: ");
  Serial.println(BLE.address());
  Serial.println("XIAO nRF52840 Peripheral");
}

void setup()
{
  Serial.begin(115200);
  initBLE();
}

void loop()
{
  // Serial.println("BLE Ready");
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice client = BLE.central();

  // if a device is connected:
  if (client)
  {
    Serial.print("Connected to device: ");
    // print the device's MAC address:
    Serial.println(client.address());

    // while the device is connected send IMU data
    while (client.connected())
    {
      // TODO: when connecting all scripts - here the motion recognition starts
      //! Here the Motion Recognition later takes place and the enum value will be send to the other device
      //! Check if a new value was recognized
      // new_val != last_val
      // instead of if(true)...
      if (true)
      { // check for new data
        imuCharacteristic.setValue(CMD[5]);
        delay(1000);
      }
    }

    // when the central disconnects, print it out:
    Serial.print("Device disconnected: ");
    Serial.println(client.address());
  }
}