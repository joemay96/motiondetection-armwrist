/*
  Ciomplete Code for the Seeed Microcontroller
*/
// SEHR WICHTIG: https://forum.seeedstudio.com/t/xiao-ble-sense-in-deep-sleep-mode/263477/116

#define ARDUINO_SEEED_XIAO_NRF52840_SENSE
#include "main.h"
// #include "SdFat.h"
// #include <Adafruit_FlashTransport.h>
#include "cmds.h"

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "SF1eFilter.h"

#include "model.h"

LSM6DS3 imu(I2C_MODE, 0x6A);

/* TF Model */
const float accelerationThreshold = 2.3; // threshold of significant in G's
const int numSamples = 199;

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 16 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char *GESTURES[] = {
    "up",
    "land",
    "left",
    "right",
    // "top",
    // "down"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

// Filters
SF1eFilter *aXFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aYFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *aZFilter = SF1eFilterCreate(100.0, 0.5, 0.05, 0.02);
SF1eFilter *gXFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);
SF1eFilter *gYFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);
SF1eFilter *gZFilter = SF1eFilterCreate(120.0, 0.5, 0.05, 0.02);

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
  Filter Methods
*/

void initFilters()
{
  SF1eFilterInit(aXFilter);
  SF1eFilterInit(aYFilter);
  SF1eFilterInit(aZFilter);
  SF1eFilterInit(gXFilter);
  SF1eFilterInit(gYFilter);
  SF1eFilterInit(gZFilter);
}

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

/*
  Check Motion Recognition
*/

void motion_detection()
{
  float aX, aY, aZ, gX, gY, gZ;

  // wait for significant motion
  if (samplesRead == numSamples)
  {
    // read the acceleration data
    aX = SF1eFilterDo(aXFilter, imu.readFloatAccelX());
    aY = SF1eFilterDo(aYFilter, imu.readFloatAccelY());
    aZ = SF1eFilterDo(aZFilter, imu.readFloatAccelZ());

    // sum up the absolutes
    float aSum = fabs(aX) + fabs(aY) + fabs(aZ);
    Serial.print(aSum);

    // check if it's above the threshold
    if (aSum >= accelerationThreshold)
    {
      // reset the sample read count
      samplesRead = 0;
      return;
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples)
  {
    // check if new acceleration AND gyroscope data is available
    // read the acceleration and gyroscope data
    aX = SF1eFilterDo(aXFilter, imu.readFloatAccelX());
    aY = SF1eFilterDo(aYFilter, imu.readFloatAccelY());
    aZ = SF1eFilterDo(aZFilter, imu.readFloatAccelZ());

    gX = SF1eFilterDo(gXFilter, imu.readFloatGyroX());
    gY = SF1eFilterDo(gYFilter, imu.readFloatGyroY());
    gZ = SF1eFilterDo(gZFilter, imu.readFloatGyroZ());

    // normalize the IMU data between 0 to 1 and store in the model's
    // input tensor
    tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 2.0) / 4.0;
    tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 2.0) / 4.0;
    tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 2.0) / 4.0;
    tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 1000.0) / 2000.0;
    tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 1000.0) / 2000.0;
    tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 1000.0) / 2000.0;

    samplesRead++;

    if (samplesRead == numSamples)
    {
      // Run inferencing
      TfLiteStatus invokeStatus = tflInterpreter->Invoke();
      if (invokeStatus != kTfLiteOk)
      {
        Serial.println("Invoke failed!");
        while (1)
          ;
        return;
      }

      // Loop through the output tensor values from the model
      for (int i = 0; i < NUM_GESTURES; i++)
      {
        Serial.print(GESTURES[i]);
        Serial.print(": ");
        Serial.println(tflOutputTensor->data.f[i], 6);
        if (tflOutputTensor->data.f[i] >= 0.65)
        {
          // "up",
          if (i == 0)
          {
            imuCharacteristic.setValue(CMD[2]);
          }
          // "land",
          else if (i == 1)
          {
            imuCharacteristic.setValue(CMD[3]);
          }
          // "left",
          else if (i == 2)
          {
            imuCharacteristic.setValue(CMD[6]);
          }
          // "right",
          else if (i == 3)
          {
            imuCharacteristic.setValue(CMD[7]);
          }
        }
      }
    }
  }
}

void setup()
{
  Serial.begin(9600);

  /**
   * Setup Deep Sleep
   */

  while (!Serial)
    delay(10);             // for nrf52840 with native usb
  pinMode(ledPin, OUTPUT); // use the LED as an output

  // gyro möchte ich natürlich anschalten, wenn nicht im Deep Sleep ist
  // imu.settings.gyroEnabled = 0; // Gyro currently not used, disabled to save power
  if (imu.begin() != 0)
  {
    Serial.println("IMU error");
  }
  else
  {
    Serial.println("Device OK!");
  }

  // Setting up the hole interrupt for the deep sleep
  setupDoubleTapInterrupt();
  pinMode(int1Pin, INPUT); // pin 1 == RST pin
  attachInterrupt(digitalPinToInterrupt(int1Pin), int1ISR, RISING);

  /*
    TF Model
  */

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION)
  {
    Serial.println("Model schema mismatch!");
    while (1)
      ;
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  // Filters
  initFilters();

  /*
    BLE
  */
  initBLE();
}

void loop()
{
  setLED(false);
  // benötige ich im final nicht
  // Serial.print("Interrupt Counter: ");
  // Serial.println(interruptCount);

  /*
    Deep Sleep
  */
  //! seeed_deep_sleep();

  /*
    BLE Code
  */

  BLEDevice client = BLE.central();

  // if a device is connected:
  if (client)
  {
    Serial.print("Connected to device: ");
    // print the device's MAC address:
    Serial.println(client.address());
    Serial.println(client.deviceName());

    // while the device is connected send IMU data
    while (client.connected())
    {
      Serial.println("client connected");
      // TODO: when connecting all scripts - here the motion recognition starts
      //! Here the Motion Recognition later takes place and the enum value will be send to the other device
      //! Check if a new value was recognized
      // new_val != last_val
      // instead of if(true)...
      motion_detection();
      // if (true)
      // { // check for new data
      //   imuCharacteristic.setValue(CMD[1]);
      //   delay(1000);
      // }
      // auch wenn mit einem client connected ist soll er in den deep sleep fallen können
      //! seeed_deep_sleep();
      delay(100);
    }
    imuCharacteristic.setValue(CMD[3]);
    // when the central disconnects, print it out:
    Serial.print("Device disconnected: ");
    Serial.println(client.address());
  }
  Serial.println("No Client connected");
  delay(500);
}