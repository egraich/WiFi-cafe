#include "OrderManager.h"

OrderManager orderManager;

Order* OrderManager::addOrder(String name, int32_t messageID) {
    Order newOrder;
    newOrder.id = globalOrderIdCounter++;
    newOrder.name = name;
    newOrder.progress = 0;
    newOrder.messageID = messageID;
    
    newOrder.mac[0] = 0x02;
    for(int i = 1; i < 6; i++) {
        newOrder.mac[i] = random(0, 256);
    }

    orders.push_back(newOrder);
    return &orders.back(); 
}

Order* OrderManager::getOrderByMessageID(int32_t messageID) {
    for (size_t i = 0; i < orders.size(); i++) {
        if (orders[i].messageID == messageID) {
            return &orders[i];
        }
    }
    return nullptr;
}

bool OrderManager::removeOrder(int32_t messageID) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->messageID == messageID) {
            orders.erase(it);
            return true;
        }
    }
    return false;
}