#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TelegramBot.h"
#include "WiFiBeacon.h"

void setup() {
    Serial.begin(115200);
    
    // ВАЖНО: Включаем режим AP+STA
    WiFi.mode(WIFI_AP_STA); 
    WiFi.softAP("HiddenCafe", "12345678", 1, 1); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    Serial.print("Подключаюсь к Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\n[SYSTEM] Wi-Fi подключен!");
    
    setupTelegramBot();
    setupWiFiBeacons();
}

void loop() {
    tickTelegramBot();
    
    spamBeacons(); // Рассылаем пакеты в эфир раз в 100 мс!
}