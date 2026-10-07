/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief Optimize LED flashing routine via compilation-time memory alloc:
 1. Переписати “класичний Arduino blink” у Embedded C++
 * Використати enum class для стану LED (On / Off).
*  Використати constexpr для номера піну та інтервалу блимання.
*  Уникнути глобальних змінних (крім мінімально необхідних для стану/ISR).
2. Використати static const / constexpr для параметрів**
- Створити клас-конфігурацію або структуру з static const / constexpr для додаткових налаштувань.
- Повністю усунути «магічні числа», винісши їх у константи.
3. Виміряти час виконання superloop (Опційно)
- Виміряти час однієї ітерації loop() без delay().
- Виводити середній час або показники у Serial Monitor кожні 1000 ітерацій.
 */

#include <Arduino.h>

/* Static definitions */
// GPIO 0 by default 
constexpr int8_t buttonBoot = 0;
// LED left of the button 
constexpr int8_t ledLeft = 6;

/* Program state */
// Serial console
// The serial console is stateless: does the same action in the task window
// Averages over the last n loops
constexpr uint16_t loop_cycles(1000);
uint32_t loop_count(0);
uint32_t loop_total(0); 
// Serial output every n milliseconds, but only if 1000 loops were complete
constexpr uint16_t serial_task_interval(1000);
uint32_t serial_task_count(0);

// LED time delay in ms
constexpr uint16_t led_task_interval(2000); 
// Serial print time
uint32_t led_task_count(0);


// Turns on off the computer 
/* Two possible states 
* 0 is HIGH Output Voltage (Turns on)
* 1 is LOW Output Voltage (Turns off)
*/
typedef enum LEDState_t {
  LEDStateOFF,
  LEDStateON,
} LEDState_t;

static volatile LEDState_t state = LEDStateOFF;



// /**
// * @brief Changes LEDs State to flash in resonance with delay of 1 cycle
// * ІSR for on-PCB BOOT button
// */
// void buttonBootPressed(void){
//   state = LEDStateSERIAL;
// }

// /**
// * @brief Changes LEDs State to flash simulatenously
// * ІSR for external pin38 button
// */
// void buttonPullUpPressed(void){
//   state = LEDStateSYNCHRONOUS;

// }

void setup()
{
  Serial.begin(115200);
  // With INPUT_PULLUP: (In case DigitalPin -> button -> GND) 
  // idle = HIGH, press pulls it LOW.
  pinMode(buttonBoot, INPUT_PULLUP);
  pinMode(ledLeft, OUTPUT);
  
  // Edit (after commit 5d87db8c60ceb1d08626b65f535b7fc7374d7c62)
  // Implement logical switch; remove handling via interrupts
  // attachInterrupt(buttonBoot, buttonBootPressed, RISING);
  // attachInterrupt(buttonPullUp, buttonPullUpPressed, FALLING);
}

void loop()
{

  unsigned long startMillis = millis();

  // // 1 when External button is pressed
  // int UpButtonVal = digitalRead(buttonPullUp);
  // // 0  when BOOT button is pressed
  // int BootButtonVal = digitalRead(buttonBoot);
  
  
  // if (UpButtonVal && BootButtonVal){
  //   state = LEDStateSYNCHRONOUS;
  // }
  // if (!UpButtonVal && !BootButtonVal){
  //   state = LEDStateSERIAL;
  // }

  // FSM with a transfer window interval of led_task_interval

  if (startMillis - led_task_count >= led_task_interval){
      
    led_task_count = startMillis;
    switch (state){
      
      // Where to store total time passed? 

      case LEDStateOFF: {
          // will trigger on next task cycle 
          state = LEDStateON;
          // write LOW
          digitalWrite(ledLeft, LEDStateOFF);
      } break;
      case LEDStateON: {
          // will trigger on next task cycle 
          state = LEDStateOFF;
          // write HIGH
          digitalWrite(ledLeft, LEDStateON); 
      } break;
    }

  }

  //Serial print task
  if (loop_count > loop_cycles)
  {
    loop_count = 0; 
    // Output loop_cycles
    if (startMillis - serial_task_count >= serial_task_interval){
      serial_task_count = startMillis;
      Serial.printf("\n Timestamp: %d | ", startMillis);
      Serial.printf("\n State %d | ", state);
      Serial.printf("\n Avg Loop time from %d iterations: %d | ", loop_cycles, loop_total / loop_cycles);
      
    }
    loop_total = 0; 
  }

  loop_count += 1;
  unsigned long endMillis = millis();
  loop_total += startMillis - endMillis;
}
