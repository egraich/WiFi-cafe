#include <Arduino.h>
#include <WiFi.h>
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

FastBot bot(BOT_TOKEN);

bool isWaitingForName = false;
int32_t promptMessageID = 0;

// Функция-генератор HTML карточки заказа
String buildHostText(int id, String name, uint8_t progress) {
    return "Заказ <code>№" + String(id) + "</code>\n" +
           "Имя: <code>" + name + "</code>\n" +
           "Прогресс: <code>" + String(progress) + "%</code>";
}

// ЕДИНЫЙ ХЭНДЛЕР ДЛЯ ВСЕГО (Тексты + Коллбэки)
void handleMsg(FB_msg& msg) {
    
    // --- 1. ЛОВИМ НАЖАТИЯ КНОПОК (CALLBACKS) ---
    if (msg.query) {
        // Если нажали "Отменить" при вводе имени
        if (msg.data == "cancel_new") {
            isWaitingForName = false;
            bot.deleteMessage(msg.messageID, msg.chatID);
            return;
        }

        // Ищем заказ по ID сообщения-хоста
        Order* order = orderManager.getOrderByMessageID(msg.messageID);
        if (order == nullptr) return; 

        bool changed = false;

        // Тут мы уже проверяем наши скрытые коллбэки, а не эмодзи кнопок!
        if (msg.data == "btn_next" && order->progress < 100) {
            order->progress += 20; 
            changed = true;
        } 
        else if (msg.data == "btn_prev" && order->progress > 0) {
            order->progress -= 20;
            changed = true;
        }
        else if (msg.data == "btn_del") {
            orderManager.removeOrder(msg.messageID);
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("Заказ удален!");
            return;
        }

        // Если % изменился, обновляем сообщение
        if (changed) {
            String newText = buildHostText(order->id, order->name, order->progress);
            String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ";
            String cb = "btn_prev,btn_next,btn_del"; // Скрытые коллбэки
            
            // В FastBot, чтобы обновить и текст, и меню с коллбэками, 
            // мы используем связку из двух команд:
            bot.editMessage(msg.messageID, newText, msg.chatID); 
            bot.editMenuCallback(msg.messageID, kb, cb, msg.chatID);
        }
        return; // Обязательно выходим, чтобы коллбэк не пошел дальше
    }

    // --- 2. ЛОВИМ ОБЫЧНЫЕ ТЕКСТЫ ---
    if (msg.chatID != ADMIN_ID) return; // Защита от левых людей

    if (msg.text == "/new") {
        isWaitingForName = true;
        // Создаем меню с коллбэком cancel_new
        int32_t sentID = bot.inlineMenuCallback("Введите имя клиента:", "❌ Отменить заказ", "cancel_new", msg.chatID);
        promptMessageID = sentID;
        bot.deleteMessage(msg.messageID, msg.chatID); // Удаляем саму команду /new
        return;
    }

    if (isWaitingForName && msg.text != "") {
        isWaitingForName = false; // Выключаем режим ожидания

        // Чистим чат от мусора (убираем "Введи имя" и сообщение повара)
        if (promptMessageID != 0) {
            bot.deleteMessage(promptMessageID, msg.chatID);
            promptMessageID = 0;
        }
        bot.deleteMessage(msg.messageID, msg.chatID);

        // Готовим клаву и коллбэки
        String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ"; 
        String cb = "btn_prev,btn_next,btn_del";
        
        // 1. Отправляем "рыбу" сообщения (пока без ID, т.к. заказ еще не создан)
        String initialText = buildHostText(0, msg.text, 0); 
        int32_t hostMsgID = bot.inlineMenuCallback(initialText, kb, cb, msg.chatID);

        // 2. Регаем заказ в оперативке (получаем реальный ID)
        Order* newOrder = orderManager.addOrder(msg.text, hostMsgID);

        // 3. Мгновенно обновляем текст в Телеге (вставляем ID)
        String finalText = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
        bot.editMessage(hostMsgID, finalText, msg.chatID);
        bot.editMenuCallback(hostMsgID, kb, cb, msg.chatID);
        
        Serial.println("Создан заказ №" + String(newOrder->id) + " | " + newOrder->name);
    }
}

void setup() {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWi-Fi OK");
    
    bot.setTextMode(FB_HTML); // Врубаем HTML для тегов <code>
    bot.attach(handleMsg);    // Подключаем наш ЕДИНСТВЕННЫЙ универсальный хэндлер
    
    Serial.println("Бот готов! Жду команду /new");
}

void loop() {
    bot.tick();
}