/*
https://github.com/arduino/arduino-ide/issues/803
*/

#include <LSM6DS3.h>
#include <Wire.h>

//Create a instance of class LSM6DS3
LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
float aX, aY, aZ, gX, gY, gZ;
const float accelerationThreshold = 3.5;  // threshold of significant in G's
const int numSamples = 119;
int samplesRead = numSamples;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial)
    ;
  //Call .begin() to configure the IMUs
  if (IMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("aX,aY,aZ,gX,gY,gZ");
  }
}

void loop() {
  // wait for significant motion
  while (samplesRead == numSamples) {
    // read the acceleration data
    float aX = IMU.readFloatAccelX();
    float aY = IMU.readFloatAccelY();
    float aZ = IMU.readFloatAccelZ() - 1.02;  //! Zum ausgleich

    // if (aX < 0) {
    //   aX = 0;
    // }
    // if (aY < 0) {
    //   aY = 0;
    // }
    // if (aZ < 0) {
    //   aZ = 0;
    // }

    // sum up the absolutes
    float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

    // check if it's above the threshold
    if (aSum >= accelerationThreshold) {
      // reset the sample read count
      samplesRead = 0;
      break;
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if both new acceleration and gyroscope data is
    // available
    // read the acceleration and gyroscope data

    samplesRead++;

    // mache einen Timeout beim letzten Durchlauf
    if (samplesRead == numSamples) {
      delay(700);
      Serial.println("BREAK!");
      break;
    }

    // print the data in CSV format
    Serial.print(IMU.readFloatAccelX(), 3);
    Serial.print(',');
    Serial.print(IMU.readFloatAccelY(), 3);
    Serial.print(',');
    Serial.print(IMU.readFloatAccelZ() - 1.02, 3);
    Serial.print(',');
    Serial.print(IMU.readFloatGyroX() + 0.7, 3);
    Serial.print(',');
    Serial.print(IMU.readFloatGyroY() + 1.4, 3);
    Serial.print(',');
    Serial.print(IMU.readFloatGyroZ(), 3);
    Serial.println();

    if (samplesRead == numSamples) {
      // add an empty line if it's the last sample
      Serial.println();
    }
  }
}