// Includes an accelerometer and gyroscope

/* Links
https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/
https://how2electronics.com/using-imu-microphone-on-xiao-ble-nrf52840-sense/
https://wiki.seeedstudio.com/XIAO_BLE/
*/


/*****************************************************************************/
//  HighLevelExample.ino
//  Hardware:      Grove - 6-Axis Accelerometer&Gyroscope
//	Arduino IDE:   Arduino-1.65
//	Author:	       Lambor
//	Date: 	       Oct,2015
//	Version:       v1.0
//
//  Modified by:
//  Data:
//  Description:
//
//	by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/*******************************************************************************/

#include "LSM6DS3.h"
#include "Wire.h"

//Create a instance of class LSM6DS3
LSM6DS3 IMU(I2C_MODE, 0x6A);    //I2C device address 0x6A

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    while (!Serial);
    //Call .begin() to configure the IMUs
    if (IMU.begin() != 0) {
        Serial.println("Device error");
    } else {
        Serial.println("Device OK!");
    }
}

void loop() {
    //Accelerometer
    float acc_x = IMU.readFloatAccelX();
    float acc_y = IMU.readFloatAccelY();
    float acc_z = IMU.readFloatAccelZ() - 1.02;

    // if (acc_x < 0) {
    //   aX = 0;
    // }
    // if (acc_y < 0) {
    //   aY = 0;
    // }    
    // if (acc_z < 0) {
    //   aZ = 0;
    // }

    Serial.print("\nAccelerometer:\n");
    Serial.print(" X1 = ");
    Serial.println(acc_x, 4);
    Serial.print(" Y1 = ");
    Serial.println(acc_y, 4);
    Serial.print(" Z1 = ");
    Serial.println(acc_z, 4);

    //Gyroscope
    float gyr_x = IMU.readFloatGyroX() + 0.7;
    float gyr_y = IMU.readFloatGyroY() + 1.4;
    float gyr_z = IMU.readFloatGyroZ();

    Serial.print("\nGyroscope:\n");
    Serial.print(" X1 = ");
    Serial.println(gyr_x, 4);
    Serial.print(" Y1 = ");
    Serial.println(gyr_y, 4);
    Serial.print(" Z1 = ");
    Serial.println(gyr_z, 4);

    delay(250);
}
