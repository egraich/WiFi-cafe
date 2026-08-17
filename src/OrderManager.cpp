#include "OrderManager.h"

// Создаем сам объект менеджера
OrderManager orderManager;

Order* OrderManager::addOrder(String name, int32_t messageID) {
    Order newOrder;
    newOrder.id = globalOrderIdCounter++;
    newOrder.name = name;
    newOrder.progress = 0;
    newOrder.messageID = messageID;
    
    // Заглушка для MAC-адреса. Потом сделаем так, чтобы последний байт был = id
    for(int i=0; i<6; i++) newOrder.mac[i] = 0;

    orders.push_back(newOrder);
    
    // Возвращаем указатель на последний добавленный элемент (полезно для вывода)
    return &orders.back(); 
}

Order* OrderManager::getOrderByMessageID(int32_t messageID) {
    // Бежим по вектору. Возвращаем указатель на заказ, если нашли
    for (size_t i = 0; i < orders.size(); i++) {
        if (orders[i].messageID == messageID) {
            return &orders[i];
        }
    }
    return nullptr; // Если не нашли
}

bool OrderManager::removeOrder(int32_t messageID) {
    // В C++ удаление из вектора делается через итераторы
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->messageID == messageID) {
            orders.erase(it);
            return true;
        }
    }
    return false;
}