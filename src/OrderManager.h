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
    Order* addOrder(String name, int32_t messageID);
    Order* getOrderByMessageID(int32_t messageID);
    bool removeOrder(int32_t messageID);
    std::vector<Order>& getAllOrders() { return orders; }
};

extern OrderManager orderManager;