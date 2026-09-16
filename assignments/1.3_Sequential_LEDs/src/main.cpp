/**
 * @file    main.cpp
 * @brief   Starting point for an ESP32-S3-N16R8 assignment.
 *
 * Blinks the on-board LED and prints a heartbeat. Replace the body of loop()
 * with the assignment's logic.
 *
 * Serial output goes to BOTH ports on purpose:
 *   Serial  - native USB CDC. Convenient, but the USB device re-enumerates on
 *             every reset, so the host terminal reattaches seconds after
 *             setup() has already run: boot banners and early crashes are lost.
 *   Serial0 - UART0, wired to the CH343 bridge. Stays up across resets and
 *             also carries the ROM bootloader log and panic backtraces.
 */

#include <Arduino.h>

/* DevKitC-1 has an addressable RGB LED on GPIO48. If your board exposes a
 * plain LED instead, point LED_PIN at that GPIO. */
#ifndef LED_PIN
#define LED_PIN 48
#endif

/* Mirror every message to both links. */
#define LOG(...)                    \
    do                              \
    {                               \
        Serial.printf(__VA_ARGS__); \
        Serial0.printf(__VA_ARGS__);\
    } while (0)

static uint32_t tick = 0;

void setup()
{
    Serial.begin(115200);   /* native USB CDC */
    Serial0.begin(115200);  /* UART0 -> CH343 port */

    /* Give the USB CDC host a chance to attach. The UART copy is already
     * flowing, so this is a convenience, not a dependency. */
    while (!Serial && millis() < 5000)
    {
    }

    pinMode(LED_PIN, OUTPUT);

    LOG("\n=== EMB_25 :: ESP32-S3-N16R8 ===\n");
    LOG("Flash : %u MB\n", (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
    LOG("PSRAM : %u bytes free\n", (unsigned)ESP.getFreePsram());
    LOG("Heap  : %u bytes free\n", (unsigned)ESP.getFreeHeap());
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(900);

    LOG("tick %lu\n", (unsigned long)(++tick));
}
