#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TelegramBot.h"
#include "WiFiBeacon.h"

/** Hardware and network setup routine. */
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    WiFi.mode(WIFI_AP_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    while (WiFi.status() != WL_CONNECTED) { 
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
    
    digitalWrite(LED_BUILTIN, LOW);

    setupTelegramBot();
    setupWiFiBeacons();

    delay(100);
    sendStartupNotification();
}

/** Main execution loop. */
void loop() {
    tickTelegramBot();
    spamBeacons();
}