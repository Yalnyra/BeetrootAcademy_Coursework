/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief   Pull-up, pull-down and floating pin demo, via button activating an LED
 * Pull-up Button - Externally attached to a GPIO pin, Outputs HIGH on button press
 * BOOT - Internal button, already attached in pull-down mode to the screen 
 * Two flashing modes 
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

/* Static definitions */
const int buttonPullUp = 38;
const int buttonBoot = 0;
// LED left of the button 
const int ledLeft = 4;
// LED right of the button 
const int ledRight = 5;


typedef enum LEDState_t {
  LEDStateSYNCHRONOUS,
  LEDStateSERIAL,
} LEDState_t;

/* Program state */
unsigned long previousMillis = 0;
const long interval = 50;
// Controls to which state the button is held 
static volatile LEDState_t state = LEDStateSYNCHRONOUS;

void buttonBootPressed(void){
  state = LEDStateSERIAL;
}

/**
* @brief ISR for pressing a button 
*/
void buttonPullUpPressed(void){
  state = LEDStateSYNCHRONOUS;

}

void setup()
{
  Serial.begin(9600);
  pinMode(buttonPullUp, INPUT_PULLUP);
  pinMode(buttonBoot, INPUT_PULLUP);

  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);

  // In ESP32 Arduino, the GPIO number is the interrupt ID; No mapping needed
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
