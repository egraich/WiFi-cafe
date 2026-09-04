## Inspiration
Traditional queue management systems in cafes require downloading custom apps, scanning QR codes, or constantly refreshing heavy browser pages. This creates friction for customers who just want to know when their order is ready. I wanted to create a zero-friction, zero-install system that fits into the native smartphone UI.

## What it does
Wi-Fi Cafe broadcasts real-time order readiness updates directly into the guest's native smartphone Wi-Fi settings menu. Customers don't need to connect to any network, download apps, or create accounts. They just open their phone's Wi-Fi list and see their order progress as a network name (e.g., "Alex: 40%").

## How I built it
The project runs entirely serverless on an ESP32 board. I wrote the firmware in C++ using the PlatformIO IDE. To broadcast dynamic multi-SSIDs, I used low-level ESP-IDF networking APIs (`esp_wifi_80211_tx`) to inject raw 802.11 beacon management frames byte-by-byte. For the management backend, I integrated the FastBot library to build an asynchronous Telegram Bot UI for the kitchen staff, operating the chip in a concurrent `WIFI_AP_STA` mode.

## Challenges I ran into
The biggest challenge I faced was managing the single ESP32 radio antenna. To prevent system crashes and packet drops, I had to force and lock both the live Telegram HTTPS connection and the raw beacon packet injection onto the exact same radio channel (Channel 7). Optimizing the asynchronous Telegram bot to parse commands instantly without freezing the beacon spam loops on limited hardware was also a tough task.

## Accomplishments that I'm proud of
I managed to implement low-level 802.11 frame crafting and manual sequence control within a compact, serverless embedded environment. The system successfully updates multiple statuses concurrently without any lags on the staff dashboard side.

## What I learned
I got deep into the 802.11 MAC layer protocol structure and learned how to mix high-level Arduino libraries with native low-level ESP-IDF SDK functions inside PlatformIO to bypass hardware abstraction limitations.

## What's next for Wi-Fi Cafe
* **Fixing OS Caching:** Implementing active Probe Responses to handle iOS aggressive Wi-Fi list caching and eliminate duplicate network entries.
* **Non-Volatile Memory (NVM) Fail-Safe:** Integrating the `Preferences` or `LittleFS` library to save active orders into the ESP32's flash memory. If the power bank accidentally disconnects, all active orders will automatically restore on reboot.
* **Interactive Captive Portal:** Setting up an offline DNS/HTML captive portal. If a guest clicks on their "Alex: 100%" network out of curiosity, it will open a local web page hosted on the ESP32 saying "Your coffee is ready! Pick it up at Counter #2" with some CSS effects.
* **Local Web Dashboard (Offline Mode):** For large open-air festivals without cell service where Telegram doesn't work, hosting a local Web Server on the chip. The staff can connect to the hidden network, open `192.168.4.1`, and manage orders via a local web interface.
* **Migrating to FastBot2:** Upgrading the Telegram library framework to leverage persistent Keep-Alive HTTP connections for near zero-latency button response times.