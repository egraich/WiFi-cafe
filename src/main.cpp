#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TelegramBot.h"
#include "WiFiBeacon.h"

/** Hardware and network setup routine. */
void setup() {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_AP_STA); 
    WiFi.softAP("HiddenCafe", "12345678", 1, 1); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.print("[SYSTEM] Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print("."); 
    }
    Serial.println("\n[SYSTEM] Wi-Fi connected! IP: " + WiFi.localIP().toString());
    
    setupTelegramBot();
    setupWiFiBeacons();
}

/** Main execution loop. */
void loop() {
    tickTelegramBot();
    spamBeacons();
}