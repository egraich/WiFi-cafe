# Wi-Fi Cafe

An interactive, zero-app queue management system that broadcasts order progress directly to guests' smartphones via dynamic Wi-Fi SSIDs, controlled via a private Telegram bot on an ESP32.

<img width="358" height="153" alt="image" src="https://github.com/user-attachments/assets/748f7739-d181-4a86-982e-602e737456e3" />

**[Watch demo video on YouTube](https://youtu.be/sZdIltr857k)**

## Quick Start

Flash the firmware to your ESP32 in three commands:

```bash
git clone https://github.com/egraich/wifi-cafe.git
cd wifi-cafe
pio run --target upload
```

Configure your credentials in `src/Config.h`, power the board from a powerbank, and enable your phone's hotspot. The onboard LED will turn off and the bot will send a ready message to Telegram once connected.

## Features

* **Zero-Install Client Interface:** Guests check their order status directly from their phone's native Wi-Fi settings menu (e.g., `Alex: 40%`) with no apps, logins, or QR code scans.
* **Chefs' Telegram Controller:** Manage queue items using interactive inline buttons (`[ ⬅️ ]`, `[ ➡️ ]`, `[ 🗑 DELETE ]`) with automated chat cleanup (FSM).
* **Fast Order Creation:** Supports instant order injection via `/new <Name>` or guided creation via `/new`.
* **Fully Portable:** Runs on a single ESP32 powered by an ordinary USB powerbank and tethered to a mobile hotspot.

## How to Run It Locally

### Prerequisites

* VS Code with the [PlatformIO IDE extension](https://platformio.org/).
* ESP32 Development Board (e.g., ESP32 Dev Module / NodeMCU-32S).
* Smartphone with Wi-Fi hotspot capability.

### Configuration

Create `src/Config.h` (or copy from a template) with your network and Telegram credentials:

```cpp
#pragma once

constexpr const char* WIFI_SSID = "YOUR_HOTSPOT_SSID";
constexpr const char* WIFI_PASS = "YOUR_HOTSPOT_PASSWORD";
constexpr const char* BOT_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN";
constexpr const char* ADMIN_ID  = "YOUR_TELEGRAM_USER_ID";

#ifndef LED_BUILTIN
constexpr uint8_t LED_BUILTIN = 2;
#endif
```

### Build & Upload

1. Open the project folder in VS Code / PlatformIO.
2. Connect your ESP32 board via USB.
3. Build and flash the project:

```bash
pio run --target upload
```

4. Enable your mobile hotspot using the credentials set in `Config.h`.
5. The onboard status LED will blink every 400 ms while negotiating the Wi-Fi connection and shut off once ready.

## How It Works

The project combines simultaneous client connectivity with raw 802.11 frame injection on a single radio channel.

```
                  ┌───────────────────────────────┐
                  │          Smartphone           │
                  │   (Hotspot + Telegram App)    │
                  └───────┬───────────────▲───────┘
                          │               │
      Internet Connection │               │ Telegram Bot API
               (WIFI_STA) │               │ (FastBot)
                          ▼               │
                  ┌───────────────────────┴───────┐
                  │             ESP32             │
                  │  List of Orders (std::vector) │
                  └───────────────┬───────────────┘
                                  │
                                  │ Raw 802.11 Beacon Frames
                                  │ (esp_wifi_80211_tx)
                                  ▼
                  ┌───────────────────────────────┐
                  │    Guests' Mobile Devices     │
                  │  (Wi-Fi List: "Alex: 60%")    │
                  └───────────────────────────────┘
```

### Dual-Interface Network Operation (AP + STA)
The ESP32 operates in `WIFI_AP_STA` mode. It connects as a Station (STA) to the mobile hotspot to handle HTTPS long-polling to the Telegram Bot API. Concurrently, a hidden SoftAP interface is initialized to obtain an active transmit handle (`WIFI_IF_AP`) on the exact same radio channel.

### Raw 802.11 Beacon Injection
Standard SDKs only support hosting a single SSID. To overcome this, the firmware uses the low-level ESP-IDF `<esp_wifi.h>` API (`esp_wifi_80211_tx`) to craft and broadcast raw 802.11 management beacon frames every 100 ms for each active order. The payload contains dynamically formatted SSID tags (`Name: XX%`), supported rate definitions, and DS parameter sets.

### Memory & Execution Budget
The active order list is stored in RAM using `std::vector<Order>`. Progress values use `uint8_t` (0–100%) to conserve SRAM, and strings are constrained to the standard 32-byte 802.11 SSID length limit with automated suffix byte reservation (`: 100%`).

## Credits

* **[FastBot](https://github.com/GyverLibs/FastBot)** by AlexGyver — An exceptionally lightweight, non-blocking Telegram Bot library for ESP32/ESP8266. Its zero-overhead polling and built-in inline keyboard/callback routing made building responsive UI cards on embedded hardware seamless.
* **Espressif Systems** — For providing raw 802.11 packet transmission capabilities within the ESP-IDF networking stack.

[Read Project Story](https://github.com/egraich/WiFi-cafe/blob/main/docs/ProjectStory.md)

Made by [egraich](https://egraich.dev) <3