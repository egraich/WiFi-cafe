#include <Arduino.h>
#include <WiFi.h>
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

FastBot bot(BOT_TOKEN);

bool isWaitingForName = false;
int32_t promptMessageID = 0;

// Генератор HTML карточки заказа
String buildHostText(int id, String name, uint8_t progress) {
    return "Заказ <code>№" + String(id) + "</code>\n" +
           "Имя: <code>" + name + "</code>\n" +
           "Прогресс: <code>" + String(progress) + "%</code>";
}

void handleMsg(FB_msg& msg) {
    
    // ==========================================
    // 1. ЛОВИМ НАЖАТИЯ КНОПОК (CALLBACKS)
    // ==========================================
    if (msg.query) {
        
        // --- Кнопка "Отменить заказ" ---
        if (msg.data == "cancel_new") {
            isWaitingForName = false;
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[FSM] Нажата отмена. Сообщение удалено.");
            return;
        }

        // --- Управление заказом ---
        Order* order = orderManager.getOrderByMessageID(msg.messageID);
        if (order == nullptr) {
            Serial.println("[ERROR] Коллбэк от старого сообщения (заказ не найден в памяти). Игнорирую.");
            return; 
        }

        bool changed = false;

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
            Serial.println("[ORDER] Заказ №" + String(order->id) + " удален из памяти и чата.");
            return;
        }

        // Если процент изменился - обновляем
        if (changed) {
            String newText = buildHostText(order->id, order->name, order->progress);
            String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ";
            String cb = "btn_prev,btn_next,btn_del";
            
            // В FastBot нет единой функции редактирования всего, поэтому шлем 2 запроса
            bot.editMessage(msg.messageID, newText, msg.chatID); 
            bot.editMenuCallback(msg.messageID, kb, cb, msg.chatID);
            
            Serial.println("[UPDATE] Заказ №" + String(order->id) + " | Прогресс: " + String(order->progress) + "%");
        }
        return; 
    }

    // ==========================================
    // 2. ЛОВИМ ТЕКСТ (КОМАНДЫ И ИМЕНА)
    // ==========================================
    if (msg.chatID != ADMIN_ID) return;

    // --- Команда /new ---
    if (msg.text == "/new") {
        isWaitingForName = true;
        
        // Отправляем меню
        bot.inlineMenuCallback("Введи имя клиента:", "❌ Отменить заказ", "cancel_new", msg.chatID);
        
        // ПРАВИЛЬНЫЙ захват ID отправленного сообщения!
        promptMessageID = bot.lastBotMsg();
        
        bot.deleteMessage(msg.messageID, msg.chatID); // сносим /new
        Serial.println("[FSM] Команда /new. Жду ввода имени... (ID запроса: " + String(promptMessageID) + ")");
        return;
    }

    // --- Ловим имя клиента ---
    if (isWaitingForName && msg.text != "") {
        Serial.println("[FSM] Получено имя: " + msg.text);
        isWaitingForName = false; 

        // 1. Создаем заказ в памяти, НО пока передаем 0 вместо ID сообщения
        Order* newOrder = orderManager.addOrder(msg.text, 0);

        // 2. Сразу готовим финальный красивый текст
        String finalText = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
        String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ"; 
        String cb = "btn_prev,btn_next,btn_del";
        
        // 3. Отправляем готовое сообщение-хост ОДНИМ запросом (это мгновенно!)
        bot.inlineMenuCallback(finalText, kb, cb, msg.chatID);
        int32_t hostMsgID = bot.lastBotMsg(); // Ловим его реальный ID
        
        // 4. Записываем ID в структуру заказа
        newOrder->messageID = hostMsgID;
        Serial.println("[ORDER] Создан заказ №" + String(newOrder->id) + ". Привязан к сообщению: " + String(hostMsgID));

        // 5. И только теперь чистим мусор в фоне (юзер уже видит карточку заказа)
        if (promptMessageID != 0) {
            bot.deleteMessage(promptMessageID, msg.chatID);
            promptMessageID = 0;
        }
        bot.deleteMessage(msg.messageID, msg.chatID);
    }
}

void setup() {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Подключаюсь к Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\n[SYSTEM] Wi-Fi подключен!");
    
    bot.setTextMode(FB_HTML); 
    bot.attach(handleMsg);    
    
    Serial.println("[SYSTEM] Бот запущен и готов рвать! Жду /new");
}

void loop() {
    bot.tick();
}