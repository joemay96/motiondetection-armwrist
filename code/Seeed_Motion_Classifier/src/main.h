#ifndef MAIN_H
#define MAIN_H

#define ARDUINO_SEEED_XIAO_NRF52840_SENSE
#include <LSM6DS3.h>
// #include <Wire.h>
#include <Arduino.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
// #include <tensorflow/lite/version.h>

#include "SF1eFilter.h"
// #include "model.h"
#include "model_all_1600.h"

#endif