/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief Optimize LED flashing routine via compilation-time memory alloc:
 1. Переписати “класичний Arduino blink” у Embedded C++
 * Використати enum class для стану LED (On / Off).
*  Використати static constexpr для номера піну та інтервалу блимання.
*  Уникнути глобальних змінних (крім мінімально необхідних для стану/ISR).
2. Використати static const / static constexpr для параметрів
- 
- Задати конфігурацію через структуру  з static const / static constexpr для додаткових налаштувань.
- Повністю усунути «магічні числа», винісши їх у константи.
3. Виміряти час виконання superloop
- Виміряти час однієї ітерації loop() без delay().
- Виводити середній час або показники у Serial Monitor кожні 1000 ітерацій.
 */

#include <Arduino.h>

/* Program state */
/* Static definitions */
struct Config {
static constexpr int8_t LED_PIN = 6;
// Serial (JTAG) out
static constexpr int32_t JTAG_USB_BAUD = 115200;

static constexpr uint32_t LOOP_CYCLES = 1000;

// Serial output every n microseconds, but only if 1000 loops were complete
static constexpr uint32_t SERIAL_TASK_INTERVAL = 100000;

// LED time delay in microseconds
static constexpr uint32_t LED_TASK_INTERVAL = 200000; 

};
/* Global counters */ 
static uint32_t loop_count(0);
static uint32_t loop_total(0); 
// Serial print timestamp
static uint32_t serial_task_count(0);

// LED Blink state (TODO: Rewrite with a bool)
/* Two possible states 
* 0 is HIGH Output Voltage (Turns on)
* 1 is LOW Output Voltage (Turns off)
*/
typedef enum LEDState_t {
  LEDStateOFF,
  LEDStateON,
} LEDState_t;


// LED Class definition;
class LED {
public:

    LED(): pin(Config::LED_PIN), frequency(Config::LED_TASK_INTERVAL), state(LEDStateOFF){

    }
    // GPIO output pin constructor
    explicit LED(uint8_t pin): pin(pin), frequency(Config::LED_TASK_INTERVAL), state(LEDStateOFF){
      // GPIO pin mode by default
      pinMode(pin, OUTPUT);

    }

    // GPIO output pin constructor
    explicit LED(uint8_t pin, uint32_t frequency): pin(pin), frequency(frequency), state(LEDStateOFF){
      // GPIO pin mode by default
      pinMode(pin, OUTPUT);

    }
    ~LED(){

    }
    void init(uint8_t pin, uint32_t frequency){
      this->pin = pin;
      this->frequency = frequency;
      return;
    }
    void write_state(LEDState_t state){
      digitalWrite(this->pin, state);
      return;
    } 
    
    void set_state(LEDState_t state){
      this->state = state;
      digitalWrite(this->pin, state);
      return;
    } 

    LEDState_t get_state(void){
      return this->state;
    } 

    uint8_t get_pin(void){
      return pin;
    }

    void set_pin(LEDState_t state){
      digitalWrite(this->pin, state);
      // digitalWrite(ledPin, state);
      return;
    } 

    // @brief toggle the state 
    void toggle(){
      
      switch (this->state){
      case LEDStateOFF: {
          // will trigger on next task cycle 
          this->state = LEDStateON;
          // write LOW
          this->write_state(LEDStateOFF);
      } break;
      case LEDStateON: {
          // will trigger on next task cycle 
          this->state = LEDStateOFF;
          // write HIGH
          this->write_state(LEDStateON);
      } break;

      Serial.printf("\n State %d | ", this->state);
    }

  }
  
    uint8_t pin; 
    uint32_t frequency;
    uint32_t task_count;
    volatile LEDState_t state;
    // Set the GPIO pin mode 
    // bool pullup; TODO: add as an enum  
};

static LED led_slow = LED();

void setup()
{
  Serial.begin(Config::JTAG_USB_BAUD);
  //Init LED object
  led_slow.init(Config::LED_PIN, Config::LED_TASK_INTERVAL);
}
void loop()
{
  // New loop iteration time 
  uint32_t startMillis = micros();

  // FSM with a transfer window interval of [led_task_interval]  
  if (startMillis - led_slow.task_count  >= led_slow.frequency){
      
    led_slow.task_count = startMillis;
    led_slow.toggle();
  }

  //Serial print task
  if (loop_count > Config::LOOP_CYCLES)
  {
    loop_count = 0; 
    if (startMillis - serial_task_count >= Config::SERIAL_TASK_INTERVAL){
      serial_task_count = startMillis;
      Serial.printf("\n Timestamp: %d | ", startMillis);
      Serial.printf("\n LED State Slow %d | ", led_slow.get_state());
      // The average time is smaller than microsecond;
      // Thus to not get 0, output the total time over [LOOP_CYCLES] iterations
      Serial.printf("\n Total Loop time in Microseconds over %d iterations: %d | ", Config::LOOP_CYCLES, loop_total);
      
    }
    loop_total = 0; 
  }
  // Update the total sum of loop durations
  loop_count += 1;
  uint32_t endMillis = micros();
  uint32_t duration = endMillis - startMillis;
  loop_total += duration;

}
