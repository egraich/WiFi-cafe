#include "WiFiBeacon.h"
#include <esp_wifi.h> // Та самая низкоуровневая библиотека
#include <WiFi.h>
#include "OrderManager.h"

// Стандартная шапка Wi-Fi пакета (Beacon Frame)
uint8_t beacon_header[] = {
    0x80, 0x00,                         // 0-1: Frame Control (0x80 означает Beacon пакет)
    0x00, 0x00,                         // 2-3: Duration (Длительность)
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 4-9: Destination MAC (Broadcast - кричим всем)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10-15: Source MAC (Сюда вставим MAC заказа)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16-21: BSSID (Тоже MAC заказа)
    0x00, 0x00,                         // 22-23: Sequence Control (Номер пакета)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 24-31: Timestamp (Метка времени)
    0x64, 0x00,                         // 32-33: Beacon Interval (100 миллисекунд)
    0x01, 0x04                          // 34-35: Capability Info (Сеть без пароля)
};

static uint16_t seq_num = 0; // Глобальный счетчик пакетов

void setupWiFiBeacons() {
    Serial.println("[BEACON] Генератор пакетов готов к работе!");
}

void spamBeacons() {
    // Не блокируем код. Отправляем маяки раз в 100 миллисекунд
    static unsigned long lastSpam = 0;
    if (millis() - lastSpam < 100) return;
    lastSpam = millis();

    // Получаем базу заказов
    std::vector<Order>& orders = orderManager.getAllOrders();
    if (orders.empty()) return; // Если заказов нет - эфир чист

    // Узнаем, на каком канале сейчас сидит ESP32 (канал телефона)
    uint8_t primaryChan;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);

    // Бежим по всем заказам и генерируем для каждого свою сеть
    for (Order& order : orders) {
        // 1. Формируем неизменяемый хвост (например ": 100%" - это 6 байт)
        String suffix = ": " + String(order.progress) + "%";
        
        // 2. Делаем копию имени, чтобы не сломать оригинал в Телеге
        String safeName = order.name;
        safeName.trim(); // Удаляем случайные пробелы в начале и конце!
        
        // 3. Считаем, сколько байт у нас осталось под имя (из 32 возможных)
        int maxNameLen = 32 - suffix.length();
        
        // 4. Если имя слишком длинное - обрезаем его с конца
        if (safeName.length() > maxNameLen) {
            safeName = safeName.substring(0, maxNameLen);
        }

        // 5. Клеим идеальный SSID (он теперь ВСЕГДА не больше 32 байт)
        String ssid = safeName + suffix;
        int ssid_len = ssid.length();

        // 2. Считаем размер пакета: шапка(36) + тег SSID(2) + длина имени + хвост(13)
        int packet_size = 36 + 2 + ssid_len + 13;
        uint8_t packet[packet_size];

        // 3. Копируем шапку
        memcpy(packet, beacon_header, 36);

        // 4. Вставляем уникальный MAC заказа (в поля Source и BSSID)
        memcpy(&packet[10], order.mac, 6);
        memcpy(&packet[16], order.mac, 6);

        // 5. Вставляем номер пакета (чтобы телефоны не думали, что сеть зависла)
        packet[22] = (seq_num & 0x0F) << 4;
        packet[23] = (seq_num & 0xFF0) >> 4;

        // 6. Формируем ТЕГ 0: ИМЯ СЕТИ
        packet[36] = 0x00;           // ID тега: 0 (SSID)
        packet[37] = ssid_len;       // Длина имени
        memcpy(&packet[38], ssid.c_str(), ssid_len); // Само имя

        int tail_idx = 38 + ssid_len;
        
        // 7. Формируем ТЕГ 1: СКОРОСТИ (Без этого айфоны могут слепыми быть)
        packet[tail_idx++] = 0x01; // ID тега: 1
        packet[tail_idx++] = 0x08; // Длина: 8 байт
        packet[tail_idx++] = 0x82; packet[tail_idx++] = 0x84; 
        packet[tail_idx++] = 0x8b; packet[tail_idx++] = 0x96; 
        packet[tail_idx++] = 0x24; packet[tail_idx++] = 0x30; 
        packet[tail_idx++] = 0x48; packet[tail_idx++] = 0x6c; 

        // 8. Формируем ТЕГ 3: КАНАЛ
        packet[tail_idx++] = 0x03; // ID тега: 3
        packet[tail_idx++] = 0x01; // Длина: 1 байт
        packet[tail_idx++] = primaryChan; // ТЕКУЩИЙ КАНАЛ

        // 🚀 ОТПРАВЛЯЕМ ПАКЕТ В ВОЗДУХ! (Интерфейс AP, сам пакет, размер, флаг)
        esp_wifi_80211_tx(WIFI_IF_AP, packet, packet_size, false);
        
        seq_num++; // Увеличиваем счетчик пакетов
    }
}