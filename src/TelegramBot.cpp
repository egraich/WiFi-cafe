#include "TelegramBot.h"
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

static FastBot bot(BOT_TOKEN);
static bool isWaitingForName = false;
static int32_t promptMessageID = 0;

/** Builds formatted HTML message card text for a given order. */
static String buildHostText(int id, const String& name, uint8_t progress) {
    return "Заказ <code>№" + String(id) + "</code>\n" +
           "Имя: <code>" + name + "</code>\n" +
           "Прогресс: <code>" + String(progress) + "%</code>";
}

/** Central event dispatcher handling incoming bot messages and callback queries. */
static void handleMsg(FB_msg& msg) {
    if (msg.query) {
        if (msg.data == "cancel_new") {
            isWaitingForName = false;
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[FSM] Order creation canceled.");
            return;
        }

        Order* order = orderManager.getOrderByMessageID(msg.messageID);
        if (order == nullptr) return; 

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
            int orderId = order->id;
            orderManager.removeOrder(msg.messageID);
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[ORDER] Order #" + String(orderId) + " deleted.");
            return;
        }

        if (changed) {
            String newText = buildHostText(order->id, order->name, order->progress);
            String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ";
            String cb = "btn_prev,btn_next,btn_del";
            bot.editMessage(msg.messageID, newText, msg.chatID); 
            bot.editMenuCallback(msg.messageID, kb, cb, msg.chatID);
            Serial.println("[ORDER] Order #" + String(order->id) + " updated to " + String(order->progress) + "%");
        }
        return; 
    }

    if (msg.chatID != ADMIN_ID) return;

    if (msg.text.startsWith("/new")) {
        String inputName = msg.text.substring(4);
        inputName.trim();

        if (inputName.length() > 0) {
            Order* newOrder = orderManager.addOrder(inputName, 0);

            String finalText = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
            String kb = "⬅️ \t ➡️ \n 🗑 УДАЛИТЬ"; 
            String cb = "btn_prev,btn_next,btn_del";
            
            bot.inlineMenuCallback(finalText, kb, cb, msg.chatID);
            newOrder->messageID = bot.lastBotMsg(); 

            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[ORDER] Fast creation: Order #" + String(newOrder->id));
            return;
        } else {
            isWaitingForName = true;
            bot.inlineMenuCallback("Введи имя клиента:", "❌ Отменить заказ", "cancel_new", msg.chatID);
            promptMessageID = bot.lastBotMsg();
            bot.deleteMessage(msg.messageID, msg.chatID);
            Serial.println("[FSM] Awaiting client name...");
            return;
        }
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
        Serial.println("[ORDER] Order #" + String(newOrder->id) + " created.");
    }
}

/** Configures FastBot parameters and registers message handler. */
void setupTelegramBot() {
    bot.setTextMode(FB_HTML); 
    bot.attach(handleMsg);
    Serial.println("[SYSTEM] Telegram bot initialized.");
}

/** Drives Telegram polling loop. */
void tickTelegramBot() {
    bot.tick();
}