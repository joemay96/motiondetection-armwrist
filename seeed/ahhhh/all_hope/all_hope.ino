#include <LSM6DS3.h>
#include <Wire.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
//#include <tensorflow/lite/version.h>

#include "model.h"

const float accelerationThreshold = 1.9; // threshold of significant in G's
const int numSamples = 199;

int samplesRead = numSamples;

LSM6DS3 myIMU(I2C_MODE, 0x6A);  

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
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
  "start",
  "land",
  "left",
  "right",
  "top",
  "down"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

/*
  Deep Sleep vars
*/

// init IMU
#define int1Pin PIN_LSM6DS3TR_C_INT1 // IMU for pin 1
const int ledPin = LED_BUILTIN;      // set ledPin to on-board LED
uint8_t interruptCount = 0;          // Amount of received interrupts
uint8_t prevInterruptCount = 0;      // Interrupt Counter from last loop

/*
  BLE variables
*/

BLEService imuService(AW_BLE_SERVICE_ID);
BLEIntCharacteristic imuCharacteristic(AW_BLE_SERVICE_CHARACTERISTIC, BLERead | BLENotify);
int last_val = 0;

/*
  Deep Sleep Methods
*/

void setupDoubleTapInterrupt()
{
  uint8_t error = 0;
  uint8_t dataToWrite = 0;

  // Double Tap Config
  imu.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x60); //* Acc = 416Hz (High-Performance mode)// Turn on the accelerometer
  // imuR_XL = 416 Hz, FS_XL = 2g
  imu.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x8E);    // INTERRUPTS_ENABLE, SLOPE_FDS// Enable interrupts and tap detection on X, Y, Z-axis
  imu.writeRegister(LSM6DS3_ACC_GYRO_TAP_THS_6D, 0x85);  // Set tap threshold 8C
  imu.writeRegister(LSM6DS3_ACC_GYRO_INT_DUR2, 0x7F);    // Set Duration, Quiet and Shock time windows 7F
  imu.writeRegister(LSM6DS3_ACC_GYRO_WAKE_UP_THS, 0x80); // Single & double-tap enabled (SINGLE_DOUBLE_TAP = 1)
  imu.writeRegister(LSM6DS3_ACC_GYRO_MD1_CFG, 0x08);     // Double-tap interrupt driven to INT1 pin

  // anschalten des gyrosensors
  // imu.settings.gyroEnabled = 1;
}

// Interrupt Callback
void int1ISR()
{
  interruptCount++;
}

// Kontrollieren der LED
void setLED(bool on)
{
  // data = 1 -> LED = On
  // data = 0 -> LED = Off
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

// Turn the Seeed controller "off" into deep sleep mode
void goToPowerOff()
{
  Serial.println("Going to System OFF");
  // imu.settings.gyroEnabled = 0;
  setLED(true);
  setupDoubleTapInterrupt();
  delay(1000); // delay seems important to apply settings, before going to System OFF
  // Ensure interrupt pin from IMU is set to wake up device
  nrf_gpio_cfg_sense_input(digitalPinToInterrupt(int1Pin), NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);
  delay(4000); // Trigger System OFF
  NRF_POWER->SYSTEMOFF = 1;
  // sd_power_system_off();
}

void seeed_deep_sleep()
{
  if (interruptCount > prevInterruptCount)
  {
    Serial.println("Interrupt received!");
  }
  prevInterruptCount = interruptCount;

  if (interruptCount >= 2)
  {
    // Trigger System OFF after 5 interrupts
    goToPowerOff();
  }
}


/*
  Methods for BLE
*/

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

void setup() {
  Serial.begin(9600);
  while (!Serial);
  // LED
  pinMode(ledPin, OUTPUT);

  if (myIMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  setupDoubleTapInterrupt();
  pinMode(int1Pin, INPUT); // pin 1 == RST pin
  attachInterrupt(digitalPinToInterrupt(int1Pin), int1ISR, RISING);
  
  initBLE();

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
      aX = myIMU.readFloatAccelX();
      aY = myIMU.readFloatAccelY();
      aZ = myIMU.readFloatAccelZ();

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
      aX = myIMU.readFloatAccelX();
      aY = myIMU.readFloatAccelY();
      aZ = myIMU.readFloatAccelZ();

      gX = myIMU.readFloatGyroX();
      gY = myIMU.readFloatGyroY();
      gZ = myIMU.readFloatGyroZ();

      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 2.0) / 4.0;
      tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 2.0) / 4.0;
      tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 2.0) / 4.0;
      tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 1000.0) / 2000.0;
      tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 1000.0) / 2000.0;
      tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 1000.0) / 2000.0;

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
