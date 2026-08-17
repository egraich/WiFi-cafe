#pragma once
#include <Arduino.h>

/** Initializes Wi-Fi beacon generation subsystem. */
void setupWiFiBeacons();

/** Transmits 802.11 beacon frames for all active orders. */
void spamBeacons();