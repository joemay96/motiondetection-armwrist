/* Code for Seeed BLE information sending */

#include <ArduinoBLE.h>

SleepBreathingRadarBLE radar;
// Weiß nicht ob die UUID hier random ist oder nicht
BLEService quadroService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEStringCharacteristic quadroCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

int last_val = 0;

void setup() {
  Serial.begin(9600);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Seeed BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Seeed Studio XIAO nRF52840");
  BLE.setAdvertisedService(quadroService);

  // add the characteristic to the service
  quadroService.addCharacteristic(quadroCharacteristic);

  // add service
  BLE.addService(quadroService);

  // start advertising
  BLE.advertise();

  Serial.println("Seeed Studio XIAO nRF52840 active, waiting for connections...");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()){
       radar.recvRadarBytes();                       //Receive radar data and start processing
       if (radar.newData == true) {                  //The data is received and transferred to the new list dataMsg[]
          byte dataMsg[radar.dataLen+3] = {0x00};
          dataMsg[0] = 0x53;                         //Add the header frame as the first element of the array
          for (byte n = 0; n < radar.dataLen; n++)dataMsg[n+1] = radar.Msg[n];  //Frame-by-frame transfer
          dataMsg[radar.dataLen+1] = 0x54;
          dataMsg[radar.dataLen+2] = 0x43;
          radar.newData = false;                     //A complete set of data frames is saved
          int new_val = radar.Sleep_inf(dataMsg);    //Use radar built-in algorithm to output human motion status
          if(new_val != last_val){
            radar.OutputAssignment(new_val);
            quadroCharacteristic.setValue(radar.str);
            last_val = new_val;
          }
        }
    }

    // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    }
}