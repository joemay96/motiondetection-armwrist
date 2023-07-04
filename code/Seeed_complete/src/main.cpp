/*
  Ciomplete Code for the Seeed Microcontroller
*/
// SEHR WICHTIG: https://forum.seeedstudio.com/t/xiao-ble-sense-in-deep-sleep-mode/263477/116

#define ARDUINO_SEEED_XIAO_NRF52840_SENSE
#include "main.h"
#include "SdFat.h"
// #include <Adafruit_FlashTransport.h>
#include "cmds.h"

LSM6DS3 imu(I2C_MODE, 0x6A);

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
  // uint8_t error = 0;
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

void setup()
{
  Serial.begin(115200);

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
    Serial.println("IMU OK!");
  }

  // Setting up the hole interrupt for the deep sleep
  setupDoubleTapInterrupt();
  pinMode(int1Pin, INPUT); // pin 1 == RST pin
  attachInterrupt(digitalPinToInterrupt(int1Pin), int1ISR, RISING);

  /*
    BLE
  */
  initBLE();
}

void loop()
{
  setLED(false);
  Serial.print("Interrupt Counter: ");
  Serial.println(interruptCount);

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

  delay(500);
}