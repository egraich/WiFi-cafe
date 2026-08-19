#include "OrderManager.h"

OrderManager orderManager;

/** Adds a new order and initializes unique local MAC. */
Order* OrderManager::addOrder(const String& name, int32_t messageID) {
    Order newOrder;
    newOrder.id = globalOrderIdCounter++;
    newOrder.name = name;
    newOrder.progress = 0;
    newOrder.messageID = messageID;
    
    newOrder.mac[0] = 0x24;
    for (int i = 1; i < 6; i++) {
        newOrder.mac[i] = static_cast<uint8_t>(random(0, 256));
    }

    orders.push_back(newOrder);
    return &orders.back(); 
}

/** Searches for an order matching given Telegram message ID. */
Order* OrderManager::getOrderByMessageID(int32_t messageID) {
    for (size_t i = 0; i < orders.size(); i++) {
        if (orders[i].messageID == messageID) {
            return &orders[i];
        }
    }
    return nullptr;
}

/** Erases an order matching the message ID from the collection. */
bool OrderManager::removeOrder(int32_t messageID) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->messageID == messageID) {
            orders.erase(it);
            return true;
        }
    }
    return false;
}