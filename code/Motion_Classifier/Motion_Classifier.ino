#include <LSM6DS3.h>
#include <Wire.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
//#include <tensorflow/lite/version.h>

#include "SF1eFilter.h"
#include "model.h"

float aX, aY, aZ, gX, gY, gZ;
const float accelerationThreshold = 1.9; // threshold of significant in G's
const int numSamples = 119;

int samplesRead = numSamples;

LSM6DS3 IMU(I2C_MODE, 0x6A);  

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using

// TODO: vielleicht muss ich den Tensor ein wenig größer machen?!

constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
  "fwd",
  "back",
  "left",
  "right",
  "top",
  "down",
  "left_turn",
  "right_turn",
  "start",
  "land",
  "doubleTap",
  "flex",
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

// Filters
SF1eFilter *aXFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aYFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aZFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
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
  Serial.begin(9600);
  while (!Serial);

  if (IMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  Serial.println();

  // Init the filters
  initFilters();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {
  float aX, aY, aZ, gX, gY, gZ;

  // wait for significant motion
  while (samplesRead == numSamples) {
      // read the acceleration data
      float aX = SF1eFilterDo(aXFilter, IMU.readFloatAccelX());
      float aY = SF1eFilterDo(aYFilter, IMU.readFloatAccelY());
      float aZ = SF1eFilterDo(aZFilter, IMU.readFloatAccelZ()); 

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
    // check if new acceleration AND gyroscope data is available
      // read the acceleration and gyroscope data
      aX = SF1eFilterDo(aXFilter, IMU.readFloatAccelX());
      aY = SF1eFilterDo(aYFilter, IMU.readFloatAccelY());
      aZ = SF1eFilterDo(aZFilter, IMU.readFloatAccelZ());

      gX = SF1eFilterDo(gXFilter, IMU.readFloatGyroX());
      gY = SF1eFilterDo(gYFilter, IMU.readFloatGyroY());
      gZ = SF1eFilterDo(gZFilter, IMU.readFloatGyroZ());

      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;

      samplesRead++;

      if (samplesRead == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 6);
        }
        Serial.println();
      }
  }
}
