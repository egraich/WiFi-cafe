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
The next step is implementing active Probe Responses to handle iOS and Android Wi-Fi list aggressive caching and eliminate duplicate network entries, making the UI updates on phones near-instant.
