#pragma once
#include <Arduino.h>
#include <vector>

struct Order {
    int id;              
    String name;         
    uint8_t progress;
    int32_t messageID;   
    uint8_t mac[6];      
};

class OrderManager {
private:
    std::vector<Order> orders;
    int globalOrderIdCounter = 1;

public:
    /** Adds a new order with generated random local MAC address. */
    Order* addOrder(const String& name, int32_t messageID);

    /** Finds an order by its associated Telegram message ID. */
    Order* getOrderByMessageID(int32_t messageID);

    /** Removes an order by message ID from memory. */
    bool removeOrder(int32_t messageID);

    /** Returns reference to all active orders. */
    std::vector<Order>& getAllOrders() { return orders; }
};

extern OrderManager orderManager;