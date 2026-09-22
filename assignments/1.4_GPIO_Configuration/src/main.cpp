/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief   Control the binary state of the LED flashing routine via an external button interrupt
 * Possible to add more states via expanding the LEDState_t enum struct
 * 
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

/* Static definitions */
// VCC of pull-up button
const int buttonPullUp = 38;
// GPIO 0 by default 
const int buttonBoot = 0;
// LED left of the button 
const int ledLeft = 4;
// LED right of the button 
const int ledRight = 5;

/* Two possible states 
* 0 is Synchronous
* 1 is Sequential (off-beat)
*/
typedef enum LEDState_t {
  LEDStateSYNCHRONOUS,
  LEDStateSERIAL,
} LEDState_t;

/* Program state */
unsigned long previousMillis = 0;
const long interval = 50;
// Controls the main LED routine 
// 0 is Always default value upon reset 
static volatile LEDState_t state = LEDStateSYNCHRONOUS;



/**
* @brief Changes LEDs State to flash in resonance with delay of 1 cycle
* ІSR for on-PCB BOOT button
*/
void buttonBootPressed(void){
  state = LEDStateSERIAL;
}

/**
* @brief Changes LEDs State to flash simulatenously
* ІSR for external pin38 button
*/
void buttonPullUpPressed(void){
  state = LEDStateSYNCHRONOUS;

}

void setup()
{
  Serial.begin(9600);
  // With INPUT_PULLUP: (In case DigitalPin -> button -> GND) 
  // idle = HIGH, press pulls it LOW.
  pinMode(buttonPullUp, INPUT_PULLUP);
  pinMode(buttonBoot, INPUT_PULLUP);

  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);

  // In ESP32 Arduino, the GPIO number is the interrupt ID; No mapping needed
  // FALLING is a trigger of HIGH-to-LOW transition 
  // (Assumes transition high to low)
  attachInterrupt(buttonBoot, buttonBootPressed, FALLING);
  attachInterrupt(buttonPullUp, buttonPullUpPressed, FALLING);
}

void loop()
{

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    // TODO: Check bug: does calling an interrupt inside switch brackets 
    // cause program to enter both sync & serial if statements? 
    switch (state){
      case LEDStateSYNCHRONOUS: {
          // Handle simultaneous LED flashing
          digitalWrite(ledLeft, 1);
          digitalWrite(ledRight, 1);
          delay(200);      
          digitalWrite(ledLeft, 0);
          digitalWrite(ledRight, 0);
      } break;
      case LEDStateSERIAL: {
          // Handle light-in-the-tunnel LED flashing

          digitalWrite(ledLeft, 1);
          digitalWrite(ledRight, 0);
          delay(200);
          digitalWrite(ledLeft, 0);
          digitalWrite(ledRight, 1);
          
      } break;
      // Never should occur, but if it does, set valid state & skip
      // default: {
      //   state = LEDStateSYNCHRONOUS;
      //   delay(200);
      // } break;
    }
    // Check button press & LED state for Debugging 

    Serial.printf("State %d | ", state);

    //Reset LED state 
    delay(200);
  }
}
