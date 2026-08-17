#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TelegramBot.h"

void setup() {
    Serial.begin(115200);
    
    // 1. Подключение к Wi-Fi (получаем интернет с телефона)
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Подключаюсь к Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\n[SYSTEM] Wi-Fi подключен! IP: " + WiFi.localIP().toString());
    
    setupTelegramBot();
    
}

void loop() {
    // Крутим бота
    tickTelegramBot();
    
    // В будущем здесь будет: spamBeacons();
}