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

/* Program state */
/* Static definitions */
struct Config {
static constexpr int8_t LED_PIN_SLOW = 4;

static constexpr int8_t LED_PIN_MEDIUM = 5;

static constexpr int8_t LED_PIN_FAST = 6;
// Serial (JTAG) out
static constexpr int32_t JTAG_USB_BAUD = 115200;

static constexpr uint32_t LOOP_CYCLES = 1000;


// LED time delay in microseconds
static constexpr uint32_t LED_TASK_INTERVAL_SLOW = 2000000; 

// LED time delay in microseconds
static constexpr uint32_t LED_TASK_INTERVAL_MEDIUM = 1000000; 

// LED time delay in microseconds
static constexpr uint32_t LED_TASK_INTERVAL_HIGH = 500000; 

// Serial output every n microseconds, but only if 1000 loops were complete
static constexpr uint32_t SERIAL_TASK_INTERVAL = 2000000;
};
/* Global counters */ 
static uint32_t loop_count(0);
static uint32_t loop_total(0); 
// LED Blink state (TODO: Rewrite with a bool)
/* Two possible states 
* FALSE is LOW Output Voltage (Turns off)
* TRUE is HIGH Output Voltage (Turns on)
*/
// LED Class definition;
class Task {
public:

    Task(): frequency(Config::LED_PIN_SLOW), last(0), func(nullptr){}
    explicit Task(uint32_t frequency): pin(0), frequency(frequency), last(0), func(nullptr){}
    
    explicit Task(uint32_t frequency,  void (*const func)()): pin(0), frequency(frequency), last(0), func(func){}
    // GPIO output pin enabled 
    explicit Task(uint8_t pin, uint32_t frequency,  void (*const func)()): pin(pin), frequency(frequency), last(0), func(func){}

    ~Task(){

    }
    bool init(uint8_t pin){
      if(!pin) { return false; }
      this->pin = pin; 
      pinMode(pin, OUTPUT);
      return true;
    }
    bool execute(){
      bool valid = (bool)func;
      if (valid){
        this->func();
      }
      else{
        Serial.printf("\n WARNING: No func is set in Task, ptr to function: %d", this);
      }
      return valid;
    }

  
    uint8_t pin; 
    uint32_t frequency;
    uint32_t last;   // коли запускали востаннє
    uint32_t task_count;
    // TODO: Pass a bool state to a func using a ptr 
    // TODO: State currently due to no logic to pass 
    // volatile bool state;

    private:
    void (*func)();   // pointer to a non-led blink function, crutch to remake task as a class 

};
//Serial task

void blinkSlow()   { static bool s1; s1 = !s1; digitalWrite(Config::LED_PIN_SLOW, s1); Serial.printf("\n State of pin= %d: %d", Config::LED_PIN_SLOW, s1); }
void blinkMiddle() { static bool s2; s2 = !s2; digitalWrite(Config::LED_PIN_MEDIUM, s2); Serial.printf("\n State of pin= %d: %d", Config::LED_PIN_MEDIUM, s2); }
void blinkFast()   { static bool s3; s3 = !s3; digitalWrite(Config::LED_PIN_FAST, s3); Serial.printf("\n State of pin= %d: %d", Config::LED_PIN_FAST, s3); }
void heartbeat()   { Serial.printf("\n alive @ %lu ms | Total Loop time:  %d | Avg over %d iterations %d ", millis(), loop_total, loop_count, loop_total / loop_count); loop_total = 0; }

Task tasks[] = {
  Task(Config::LED_TASK_INTERVAL_SLOW, blinkSlow),
  Task(Config::LED_TASK_INTERVAL_MEDIUM, blinkMiddle),
  Task(Config::LED_TASK_INTERVAL_HIGH, blinkFast),
  Task(Config::SERIAL_TASK_INTERVAL, heartbeat),
};

void setup()
{
  Serial.begin(115200);  
  tasks[0].init(Config::LED_PIN_SLOW);
  tasks[1].init(Config::LED_PIN_MEDIUM);
  tasks[2].init(Config::LED_PIN_FAST);
}

void loop()
{
  // New loop iteration time 
  uint32_t now = micros();
  for (Task &t : tasks) { 
      if (now - t.last >= t.frequency) { 
        t.last = now;
        t.execute(); 
      }
    }

    if (loop_count > Config::LOOP_CYCLES)
    {
      loop_count = 0; 
    }

    loop_count += 1;
    uint32_t endMillis = micros();
    uint32_t duration = endMillis - now;
    loop_total += duration;
}
