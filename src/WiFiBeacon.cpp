#include "WiFiBeacon.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include "OrderManager.h"

static const uint8_t beacon_header[] = {
    0x80, 0x00,                         
    0x00, 0x00,                         
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,                         
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x64, 0x00,                         
    0x01, 0x04                          
};

static uint16_t seq_num = 0;

/** Initializes beacon subsystem. */
void setupWiFiBeacons() {
    WiFi.softAP("HiddenCafe", "12345678", 1, 1); 
}

/** Crafts and transmits raw 802.11 beacon packets for all active orders. */
void spamBeacons() {
    static unsigned long lastSpam = 0;
    if (millis() - lastSpam < 100) return;
    lastSpam = millis();

    std::vector<Order>& orders = orderManager.getAllOrders();
    if (orders.empty()) return;

    uint8_t primaryChan = 1;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);

    for (Order& order : orders) {
        String suffix = ": " + String(order.progress) + "%";
        String safeName = order.name;
        safeName.trim();
        
        int maxNameLen = 32 - suffix.length();
        if (safeName.length() > maxNameLen) {
            safeName = safeName.substring(0, maxNameLen);
        }

        String ssid = safeName + suffix;
        int ssid_len = ssid.length();

        int packet_size = 36 + 2 + ssid_len + 13;
        uint8_t packet[packet_size];

        memcpy(packet, beacon_header, 36);
        memcpy(&packet[10], order.mac, 6);
        memcpy(&packet[16], order.mac, 6);

        packet[22] = (seq_num & 0x0F) << 4;
        packet[23] = (seq_num & 0xFF0) >> 4;

        packet[36] = 0x00;
        packet[37] = static_cast<uint8_t>(ssid_len);
        memcpy(&packet[38], ssid.c_str(), ssid_len);

        int tail_idx = 38 + ssid_len;
        packet[tail_idx++] = 0x01;
        packet[tail_idx++] = 0x08;
        packet[tail_idx++] = 0x82; packet[tail_idx++] = 0x84; 
        packet[tail_idx++] = 0x8b; packet[tail_idx++] = 0x96; 
        packet[tail_idx++] = 0x24; packet[tail_idx++] = 0x30; 
        packet[tail_idx++] = 0x48; packet[tail_idx++] = 0x6c; 

        packet[tail_idx++] = 0x03;
        packet[tail_idx++] = 0x01;
        packet[tail_idx++] = primaryChan;

        esp_wifi_80211_tx(WIFI_IF_AP, packet, packet_size, false);
        seq_num++;
    }
}