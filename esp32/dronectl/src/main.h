#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "potensic.h"

void connectToWiFi(const char * ssid);
void WiFiEvent(WiFiEvent_t event);
void sendCMD(CMD cmd);

#endif