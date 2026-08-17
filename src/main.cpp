#include <Arduino.h>
#include <WiFi.h>
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

FastBot bot(BOT_TOKEN);

// --- ПЕРЕМЕННЫЕ СОСТОЯНИЯ (FSM) ---
bool isWaitingForName = false;   // Ждем ли мы сейчас имя от повара?
int32_t promptMessageID = 0;     // ID сообщения "Введи имя клиента:", чтобы потом его стереть

// Функция-помощник: собирает красивый HTML-текст для сообщения-хоста
String buildHostText(int id, String name, int progress) {
    return "Заказ <code>№" + String(id) + "</code>\n" +
           "Имя: <code>" + name + "</code>\n" +
           "Прогресс: <code>" + String(progress) + "%</code>";
}

// 1. Хэндлер текста
void handleNewMessage(FB_msg& msg) {
    if (msg.chatID != ADMIN_ID) return;

    // Сценарий 1: Пользователь ввел команду /new
    if (msg.text == "/new") {
        isWaitingForName = true;
        // Отправляем запрос имени и кнопку отмены
        int32_t sentID = bot.inlineMenu("Введи имя клиента:", "❌ Отменить заказ", msg.chatID);
        promptMessageID = sentID; // Запоминаем ID, чтобы снести его позже
        
        // Удаляем саму команду /new из чата для чистоты (по желанию)
        bot.deleteMessage(msg.messageID, msg.chatID); 
        return;
    }

    // Сценарий 2: Мы ждали имя, и пользователь написал текст
    if (isWaitingForName && msg.text != "") {
        isWaitingForName = false; // Сбрасываем флаг

        // 1. Удаляем сообщение "Введи имя клиента:"
        if (promptMessageID != 0) {
            bot.deleteMessage(promptMessageID, msg.chatID);
        }
        // 2. Удаляем сообщение с текстом самого пользователя (оставляем только карточки заказов)
        bot.deleteMessage(msg.messageID, msg.chatID);

        // 3. Отправляем сообщение-хост в ТГ (пока без ID, берем фейковые нули)
        String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ"; 
        String initialText = buildHostText(0, msg.text, 0); // 0 id - заглушка перед добавлением
        
        int32_t hostMsgID = bot.inlineMenu(initialText, kb, msg.chatID);

        // 4. Добавляем в оперативную память (OrderManager)
        Order* newOrder = orderManager.addOrder(msg.text, hostMsgID);

        // 5. Теперь у нас есть реальный ID (№1, №2). Обновляем текст в ТГ сразу же!
        String finalText = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
        bot.editMessage(hostMsgID, finalText, msg.chatID);
        bot.editMenu(hostMsgID, kb, "", msg.chatID);
        
        Serial.println("Создан заказ №" + String(newOrder->id) + " (" + newOrder->name + ")");
    }
}

// 2. Хэндлер кнопок
void handleCallback(FB_msg& msg) {
    // Если нажали Отмену при создании заказа
    if (msg.data == "❌ Отменить заказ") {
        isWaitingForName = false;
        bot.deleteMessage(msg.messageID, msg.chatID);
        return;
    }

    // Если это управление существующим заказом
    Order* order = orderManager.getOrderByMessageID(msg.messageID);
    if (order == nullptr) return; // Если заказа нет в памяти, игнорим

    bool changed = false; // Флаг, меняли ли мы процент (чтобы зря не дергать API телеги)

    if (msg.data == "➡️" && order->progress < 100) {
        order->progress += 20; // Шаг 20%
        changed = true;
    } 
    else if (msg.data == "⬅️" && order->progress > 0) {
        order->progress -= 20;
        changed = true;
    }
    else if (msg.data == "🗑 УДАЛИТЬ") {
        // Убиваем из памяти (чтобы перестали лететь Beacon пакеты)
        orderManager.removeOrder(msg.messageID);
        // Убиваем сообщение из Телеги
        bot.deleteMessage(msg.messageID, msg.chatID);
        Serial.println("Заказ удален!");
        return;
    }

    // Если процент изменился, обновляем сообщение в ТГ
    if (changed) {
        String newText = buildHostText(order->id, order->name, order->progress);
        String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ";
        bot.editMessage(msg.messageID, newText, kb, msg.chatID);
    }
}

void setup() {
    Serial.begin(115200);
    
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    
    // Важно! Включаем поддержку HTML тегов <code>, <b> и тд.
    bot.setTextMode(FB_HTML);
    
    bot.attach(handleNewMessage);
    bot.attachUpdate(handleCallback);
    
    Serial.println("Бот готов! Жду /new");
}

void loop() {
    bot.tick();
}