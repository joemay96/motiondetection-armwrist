/*
1€ Arduino implementation: https://gery.casiez.net/1euro/

It uses a first order low-pass filter with an adaptive cutoff frequency: at low speeds,
a low cutoff stabilizes the signal by reducing jitter, but as speed increases, the cutoff is increased to reduce lag.

2 tunable parameters. beta and fcmin.
beta is 0 at first and fcmin(min cut off) is 1 Hz. 

Note that parameters fcmin and beta have clear conceptual relationships: if high speed lag is a problem, increase beta; if slow speed jitter is a problem, decrease fcmin.

#################################

Gyroskop ist für Start der Bewegungswahrnehmung eingentlich egal. Grundsätzlich ist mehr oder weniger nur die Geschwindigkeit wichtig.

*/

#include "SF1eFilter.h"
#include <LSM6DS3.h>
#include <Wire.h>

//Create a instance of class LSM6DS3
LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
float aX, aY, aZ, gX, gY, gZ;
// TODO: Vielleicht mehrere verschiedene thresholds für alle Achsen X, Y und Z
const float accelerationThreshold = 1.85;  // threshold of significant in G's

const int numSamples = 119;
int samplesRead = numSamples;

//  Cerating the 1€ filter
// TODO: Überlegen ob die Filter hier überhaupt passen mit den Werten, sonst anpassen mit Tool:
// https://gery.casiez.net/1euro/InteractiveDemo/
SF1eFilterConfiguration aConfig = {
  120,
  1,
  0.08,
  1
};
SF1eFilterConfiguration gConfig = {
  120,
  1,
  0.08,
  1
};

// SF1eFilter *aXFilter = SF1eFilterCreate(120.0, 1.0, 0.08, 1.0);
// SF1eFilter *aYFilter = SF1eFilterCreate(120.0, 1.0, 0.08, 1.0);
// SF1eFilter *aZFilter = SF1eFilterCreate(120.0, 1.0, 0.08, 1.0);

SF1eFilter *aXFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aYFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aZFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);

// TODO: wegen der Gyro Filter Werte schauen 
SF1eFilter *gXFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);
SF1eFilter *gYFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);
SF1eFilter *gZFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);

void initFilters() 
{
  SF1eFilterInit(aXFilter);
  SF1eFilterInit(aYFilter);
  SF1eFilterInit(aZFilter);
  SF1eFilterInit(gXFilter);
  SF1eFilterInit(gYFilter);
  SF1eFilterInit(gZFilter);
}

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

  initFilters();
}

void loop() {
  // wait for significant motion
  while (samplesRead == numSamples) {
    // read the acceleration data
    // TODO: Überlegen ob ich hier wirklich die Filter überhaupt anwenden möchte
    float aX = SF1eFilterDo(aXFilter, IMU.readFloatAccelX());
    float aY = SF1eFilterDo(aYFilter, IMU.readFloatAccelY());
    float aZ = SF1eFilterDo(aZFilter, IMU.readFloatAccelZ()); 

    // float gX = SF1eFilterDo(aXFilter, IMU.readFloatGyroX());
    // float gY = SF1eFilterDo(aXFilter, IMU.readFloatGyroY() + 1.4);
    // float gZ = SF1eFilterDo(aXFilter, IMU.readFloatGyroZ());

    // sum up the absolutes
    float aSum = fabs(aX) + fabs(aY) + fabs(aZ);
    // float aSum = fabs(aX + aY + aZ);
    Serial.println(aSum);
    Serial.print(aX);
    Serial.print("\t");
    Serial.print(aY);
    Serial.print("\t");
    Serial.print(aZ);
    Serial.println();

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
    samplesRead++;


  /*
    // mache einen Timeout beim letzten Durchlauf
    if (samplesRead == numSamples) {
      // TODO: Überlegen wie ich hier das delay einbaue
      delay(700);
      Serial.println("BREAK!");
      break;
    }
  */

    // print the data in CSV format
    Serial.print(SF1eFilterDo(aXFilter, IMU.readFloatAccelX()), 3);
    Serial.print(',');
    Serial.print(SF1eFilterDo(aYFilter, IMU.readFloatAccelY()), 3);
    Serial.print(',');
    Serial.print(SF1eFilterDo(aZFilter, IMU.readFloatAccelZ()), 3);
    Serial.print(',');
    Serial.print(SF1eFilterDo(gXFilter, IMU.readFloatGyroX()), 3);
    Serial.print(',');
    Serial.print(SF1eFilterDo(gYFilter, IMU.readFloatGyroY()), 3);
    Serial.print(',');
    Serial.print(SF1eFilterDo(gZFilter, IMU.readFloatGyroZ()), 3);
    Serial.println();


    if (samplesRead == numSamples) {
      // add an empty line if it's the last sample
      Serial.println();
      delay(500);
    }
  }
}
