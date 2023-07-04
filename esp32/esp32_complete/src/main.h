#ifndef DRONE_H
#define DRONE_H

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include "BLEDevice.h"
#include "BLE.h"

#include <Wire.h>
#include "potensic.h"

void connectToWiFi(const char *ssid);
void WiFiEvent(WiFiEvent_t event);
void sendCMD(CMD cmd);

#endif