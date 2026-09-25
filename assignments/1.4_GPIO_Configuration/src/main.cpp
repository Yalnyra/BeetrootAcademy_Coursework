/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief   Control the binary state of the LED flashing routine via an external button interrupt
 * Possible to add more states via expanding the LEDState_t enum struct
 * 
 */

#include <Arduino.h>

/* Static definitions */
// VCC of pull-up button
const int buttonPullUp = 21;
// GPIO 0 by default 
const int buttonBoot = 0;
// LED left of the button 
const int ledLeft = 15;
// LED right of the button 
const int ledRight = 16;

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
  pinMode(buttonPullUp, INPUT_PULLDOWN);
  pinMode(buttonBoot, INPUT_PULLUP);

  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);

  // In ESP32 Arduino, the GPIO number is the interrupt ID; No mapping needed
  // FALLING is a trigger of HIGH-to-LOW transition 
  // (Assumes transition high to low)

  
  // Edit (after commit 5d87db8c60ceb1d08626b65f535b7fc7374d7c62)
  // Implement logical switch; remove handling via interrupts
  // attachInterrupt(buttonBoot, buttonBootPressed, RISING);
  // attachInterrupt(buttonPullUp, buttonPullUpPressed, FALLING);
}

void loop()
{

  unsigned long currentMillis = millis();

  // 1 when External button is pressed
  int UpButtonVal = digitalRead(buttonPullUp);
  // 0  when BOOT button is pressed
  int BootButtonVal = digitalRead(buttonBoot);

  
  if (UpButtonVal && BootButtonVal){
    state = LEDStateSYNCHRONOUS;
  }
  if (!UpButtonVal && !BootButtonVal){
    state = LEDStateSERIAL;
  }

  switch (state){
    case LEDStateSYNCHRONOUS: {
        // Handle simultaneous LED flashing
        digitalWrite(ledLeft, 1);
        digitalWrite(ledRight, 1);
        delay(200);      
        digitalWrite(ledLeft, 0);
        digitalWrite(ledRight, 0);
        delay(200);
    } break;
    case LEDStateSERIAL: {
        // Handle light-in-the-tunnel LED flashing

        digitalWrite(ledLeft, 1);
        digitalWrite(ledRight, 0);
        // Increase delay
        delay(1000);
        digitalWrite(ledLeft, 0);
        digitalWrite(ledRight, 1);
        delay(1000);          
    } break;
    // Never should occur, but if it does, set valid state & skip
    // default: {
    //   state = LEDStateSYNCHRONOUS;
    //   delay(200);
    // } break;
  }

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    
    // Check button press & LED state for Debugging 
    Serial.printf("\n Timestamp: %d | ", currentMillis);
    Serial.printf("\n External Button: %d | ", UpButtonVal);
    Serial.printf("\n BOOT Button: %d | ", BootButtonVal);
    Serial.printf("\n State %d | ", state);

  }
}
