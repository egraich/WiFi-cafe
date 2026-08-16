#include <Arduino.h>
#include <WiFi.h>
#include <FastBot.h>
#include "Config.h"

// Создаем объект бота
FastBot bot(BOT_TOKEN);

// Функция-обработчик входящих сообщений (аналог хэндлеров в aiogram)
void handleNewMessage(FB_msg& msg) {
    // Выводим в консоль (Serial Monitor), кто нам написал
    Serial.print("Сообщение от ");
    Serial.print(msg.username);
    Serial.print(": ");
    Serial.println(msg.text);

    // Защита от дурака: если ID не совпадает с админом - игнорим
    if (msg.chatID != ADMIN_ID) {
        bot.sendMessage("Извините, вы не повар!", msg.chatID);
        return;
    }

    // Если всё ок — отправляем эхо-ответ с цитированием (reply)
    bot.replyMessage("Повар, я получил твою команду: " + msg.text, msg.messageID, msg.chatID);
}

void setup() {
    // Запускаем последовательный порт для отладки (логи в консоль)
    Serial.begin(115200);
    Serial.println("\n--- Запуск системы Wi-Fi Cafe ---");

    // 1. Подключаемся к Wi-Fi телефона
    Serial.print("Подключение к Wi-Fi: ");
    Serial.println(WIFI_SSID);
    
    // Включаем режим клиента (Station)
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Ждем подключения, пока статус не станет WL_CONNECTED
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n[OK] Успешно подключено к Wi-Fi!");
    Serial.print("Мой IP адрес: ");
    Serial.println(WiFi.localIP()); // Выведет IP, который телефон выдал ESP32

    // 2. Настройка Telegram-бота
    bot.attach(handleNewMessage); // Указываем, какая функция обрабатывает сообщения
    Serial.println("[OK] Бот запущен и готов к работе!");
}

void loop() {
    // В loop() должен крутиться только tick() бота. 
    // Никаких delay(), иначе бот будет тормозить!
    bot.tick();
}