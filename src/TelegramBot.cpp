#include "TelegramBot.h"
#include <FastBot.h>
#include "Config.h"
#include "OrderManager.h"

static FastBot bot(BOT_TOKEN);
static bool isWaitingForName = false;
static int32_t promptMessageID = 0;

static constexpr const char* ORDER_KEYBOARD  = "⬅️ \t ➡️ \n 🗑 DELETE";
static constexpr const char* ORDER_CALLBACKS = "btn_prev,btn_next,btn_del";

/** Builds formatted HTML message card text for a given order. */
static String buildHostText(int id, const String& name, uint8_t progress) {
    return "Order <code>#" + String(id) + "</code>\n" +
           "Name: <code>" + name + "</code>\n" +
           "Progress: <code>" + String(progress) + "%</code>";
}

/** Creates order record in memory and dispatches host message to Telegram. */
static void createOrderCard(const String& name, const String& chatID) {
    Order* newOrder = orderManager.addOrder(name, 0);
    String text = buildHostText(newOrder->id, newOrder->name, newOrder->progress);
    bot.inlineMenuCallback(text, ORDER_KEYBOARD, ORDER_CALLBACKS, chatID);
    newOrder->messageID = bot.lastBotMsg();
}

/** Central event dispatcher handling incoming bot messages and callback queries. */
static void handleMsg(FB_msg& msg) {
    if (msg.query) {
        if (msg.data == "cancel_new") {
            isWaitingForName = false;
            bot.deleteMessage(msg.messageID, msg.chatID);
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
            orderManager.removeOrder(msg.messageID);
            bot.deleteMessage(msg.messageID, msg.chatID);
            return;
        }

        if (changed) {
            String newText = buildHostText(order->id, order->name, order->progress);
            bot.editMessage(msg.messageID, newText, msg.chatID); 
            bot.editMenuCallback(msg.messageID, ORDER_KEYBOARD, ORDER_CALLBACKS, msg.chatID);
        }
        return; 
    }

    if (msg.chatID != ADMIN_ID) return;

    if (msg.text.startsWith("/new")) {
        String inputName = msg.text.substring(4);
        inputName.trim();

        if (inputName.length() > 0) {
            createOrderCard(inputName, msg.chatID);
            bot.deleteMessage(msg.messageID, msg.chatID);
            return;
        } else {
            isWaitingForName = true;
            bot.inlineMenuCallback("Enter client name:", "❌ Cancel", "cancel_new", msg.chatID);
            promptMessageID = bot.lastBotMsg();
            bot.deleteMessage(msg.messageID, msg.chatID);
            return;
        }
    }

    if (isWaitingForName && msg.text != "") {
        isWaitingForName = false; 
        createOrderCard(msg.text, msg.chatID);

        if (promptMessageID != 0) {
            bot.deleteMessage(promptMessageID, msg.chatID);
            promptMessageID = 0;
        }
        bot.deleteMessage(msg.messageID, msg.chatID);
    }
}

/** Configures FastBot parameters and registers message handler. */
void setupTelegramBot() {
    bot.setTextMode(FB_HTML); 
    bot.attach(handleMsg);
}

/** Handles Telegram polling loop. */
void tickTelegramBot() {
    bot.tick();
}

/** Sends startup notification to administrator. */
void sendStartupNotification() {
    bot.sendMessage("Wi-Fi connected, Wi-Fi Cafe is ready!", ADMIN_ID);
}