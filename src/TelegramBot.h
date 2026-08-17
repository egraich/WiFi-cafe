#pragma once
#include <Arduino.h>

/** Initializes FastBot parameters, text format, and handlers. */
void setupTelegramBot();

/** Handles periodic polling for incoming Telegram updates. */
void tickTelegramBot();

/** Sends system startup message to administrator. */
void sendStartupNotification();