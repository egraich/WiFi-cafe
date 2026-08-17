#pragma once
#include <Arduino.h>
#include <vector>

// Структура одного заказа
struct Order {
    int id;              // Уникальный номер заказа (№1, №2...)
    String name;         // Имя (поддерживает кириллицу)
    int progress;        // От 0 до 100
    int32_t messageID;   // ID сообщения-хоста в ТГ, к которому привязан заказ
    uint8_t mac[6];      // MAC-адрес для будущих Wi-Fi маяков
};

// Класс-менеджер для управления вектором
class OrderManager {
private:
    std::vector<Order> orders; // Наш динамический массив, скрыт от чужих глаз
    int globalOrderIdCounter = 1; // Счетчик для выдачи номеров (№1, №2...)

public:
    // Будущая функция добавления
    Order* addOrder(String name, int32_t messageID);
    
    // Получить заказ по ID сообщения (чтобы менять процент)
    Order* getOrderByMessageID(int32_t messageID);
    
    // Удалить заказ из памяти
    bool removeOrder(int32_t messageID);

    // Функция для будущего генератора Wi-Fi (вернет ссылку на массив)
    std::vector<Order>& getAllOrders() { return orders; }
};

// Объявляем глобальную переменную менеджера, чтобы main.cpp её видел
extern OrderManager orderManager;