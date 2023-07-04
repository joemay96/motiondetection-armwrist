#ifndef DRONE_H
#define DRONE_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include "potensic.h"

void connectToWiFi(const char *ssid);
void WiFiEvent(WiFiEvent_t event);
void sendCMD(CMD cmd);

#endif