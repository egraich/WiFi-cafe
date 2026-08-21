#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TelegramBot.h"
#include "WiFiBeacon.h"

static unsigned long lastBlinkTime = 0;
static unsigned long lastWifiRetry = 0;
static bool ledState = LOW;
static bool wasConnected = false;

/** Hardware and network setup routine. */
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    WiFi.mode(WIFI_AP_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    setupTelegramBot();
    setupWiFiBeacons();
}

/** Main execution loop. */
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastBlinkTime >= 200) {
            lastBlinkTime = millis();
            ledState = !ledState;
            digitalWrite(LED_BUILTIN, ledState);
        }

        wasConnected = false;

        if (millis() - lastWifiRetry >= 10000) {
            lastWifiRetry = millis();
            WiFi.reconnect();
        }
    } else {
        if (!wasConnected) {
            digitalWrite(LED_BUILTIN, LOW);
            wasConnected = true;
            lastWifiRetry = millis(); 
            sendStartupNotification();
        }
        
        tickTelegramBot();
    }
    spamBeacons();
}