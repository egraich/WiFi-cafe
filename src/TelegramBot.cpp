#include "TelegramBot.h"
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

// Бот теперь живет тут
FastBot bot(BOT_TOKEN);

bool isWaitingForName = false;
int32_t promptMessageID = 0;

// Генератор HTML карточки заказа
String buildHostText(int id, String name, uint8_t progress) {
    return "Заказ <code>№" + String(id) + "</code>\n" +
           "Имя: <code>" + name + "</code>\n" +
           "Прогресс: <code>" + String(progress) + "%</code>";
}

// Тот самый хэндлер
void handleMsg(FB_msg& msg) {
    if (msg.query) {
        if (msg.data == "cancel_new") {
            isWaitingForName = false;
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[FSM] Нажата отмена. Сообщение удалено.");
            return;
        }

        Order* order = orderManager.getOrderByMessageID(msg.messageID);
        if (order == nullptr) return; 

        bool changed = false;
        if (msg.data == "btn_next" && order->progress < 100) { order->progress += 20; changed = true; } 
        else if (msg.data == "btn_prev" && order->progress > 0) { order->progress -= 20; changed = true; }
        else if (msg.data == "btn_del") {
            orderManager.removeOrder(msg.messageID);
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[ORDER] Заказ №" + String(order->id) + " удален.");
            return;
        }

        if (changed) {
            String newText = buildHostText(order->id, order->name, order->progress);
            String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ";
            String cb = "btn_prev,btn_next,btn_del";
            bot.editMessage(msg.messageID, newText, msg.chatID); 
            bot.editMenuCallback(msg.messageID, kb, cb, msg.chatID);
        }
        return; 
    }

    if (msg.chatID != ADMIN_ID) return;

    if (msg.text == "/new") {
        isWaitingForName = true;
        bot.inlineMenuCallback("Введи имя клиента:", "❌ Отменить заказ", "cancel_new", msg.chatID);
        promptMessageID = bot.lastBotMsg();
        bot.deleteMessage(msg.messageID, msg.chatID);
        return;
    }

    if (isWaitingForName && msg.text != "") {
        isWaitingForName = false; 
        Order* newOrder = orderManager.addOrder(msg.text, 0);

        String finalText = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
        String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ"; 
        String cb = "btn_prev,btn_next,btn_del";
        
        bot.inlineMenuCallback(finalText, kb, cb, msg.chatID);
        newOrder->messageID = bot.lastBotMsg(); 

        if (promptMessageID != 0) {
            bot.deleteMessage(promptMessageID, msg.chatID);
            promptMessageID = 0;
        }
        bot.deleteMessage(msg.messageID, msg.chatID);
    }
}

// Реализация функций, которые мы объявили в .h файле
void setupTelegramBot() {
    bot.setTextMode(FB_HTML); 
    bot.attach(handleMsg);
    Serial.println("[SYSTEM] Telegram-бот инициализирован.");
}

void tickTelegramBot() {
    bot.tick();
}