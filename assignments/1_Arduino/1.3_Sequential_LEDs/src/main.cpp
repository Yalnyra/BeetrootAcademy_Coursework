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
#include <Adafruit_NeoPixel.h>

/* DevKitC-1 has an addressable RGB LED on GPIO48. If your board exposes a
 * plain LED instead, point LED_PIN at that GPIO. */
    #ifndef LED_GPIO_48
    #define LED_GPIO_48 48
    #endif
    # define LED_LENGTH 1

    #ifndef LED_OUT_RED
    #define LED_OUT_RED 15
    #endif 

    #define TOUCH_PIN 1 
    #define TOUCH_TRESHOLD 35000



/* Mirror every message to both links. */
#define LOG(...)                    \
    do                              \
    {                               \
        Serial.printf(__VA_ARGS__); \
    } while (0)

// static uint32_t tick = 0;

static uint32_t touchValue = 0;

// Address-defined LED pin on the board
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
void setup()
{
    // Serial.begin(115200);   /* native USB CDC */
    Serial0.begin(115200);  /* UART0 -> CH343 port */

    /* Give the USB CDC host a chance to attach. The UART copy is already
     * flowing, so this is a convenience, not a dependency. */
    // while (!Serial && millis() < 5000)
    // {
    // }

    // 1. Setup external LED blink node
    // delay(100);
    // // Disable everything except the LED pin
    // pinMode(LED_OUT_RED, OUTPUT);

    //2. Custom 
    pixels.begin();
    pixels.setBrightness(50);
    pixels.clear();
    pixels.show();

    LOG("\n=== EMB_25 :: ESP32-S3-N16R8 ===\n");
    LOG("Flash : %u MB\n", (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
    LOG("PSRAM : %u bytes free\n", (unsigned)ESP.getFreePsram());
    LOG("Heap  : %u bytes free\n", (unsigned)ESP.getFreeHeap());
}

void loop()
{
    // 1. Read floating digital input, light up LED on capacitance overflow
    // touchValue = touchRead(TOUCH_PIN);

    // if (touchValue > TOUCH_TRESHOLD ){
    //     LOG("Touch detected, capacity %lu\n", touchValue);
    //     digitalWrite(LED_OUT_RED, HIGH);
    //     delay(500);
    //     digitalWrite(LED_OUT_RED, LOW);
    //     // delay(1000);
    // }
    // LOG("tick %lu\n", (unsigned long)(++tick));

    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    delay(500);

  // pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  // pixels.show();
  // delay(500);

  // pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  // pixels.show();
  // delay(500);

  // pixels.setPixelColor(0, pixels.Color(255, 100, 0));
  // pixels.show();
  // delay(500);

pixels.clear();
pixels.show();
delay(1000);
}
