/**
 * @file    main.cpp
 * @brief   Starting point for an ESP32-S3-N16R8 assignment.
 *
 * Blinks the on-board LED and prints a heartbeat over the native USB serial
 * port. Replace the body of loop() with the assignment's logic.
 */

#include <Arduino.h>

/* DevKitC-1 has an addressable RGB LED on GPIO48. If your board exposes a
 * plain LED instead, point LED_PIN at that GPIO. */
#ifndef LED_PIN
#define LED_PIN 48
#endif

static uint32_t tick = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000)
    {
        /* Wait briefly for the USB CDC host to attach. */
    }

    pinMode(LED_PIN, OUTPUT);

    Serial.println();
    Serial.println("=== EMB_25 :: ESP32-S3-N16R8 ===");
    Serial.printf("Flash : %u MB\n", (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
    Serial.printf("PSRAM : %u bytes free\n", (unsigned)ESP.getFreePsram());
    Serial.printf("Heap  : %u bytes free\n", (unsigned)ESP.getFreeHeap());
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(900);

    Serial.printf("tick %lu\n", (unsigned long)(++tick));
}
