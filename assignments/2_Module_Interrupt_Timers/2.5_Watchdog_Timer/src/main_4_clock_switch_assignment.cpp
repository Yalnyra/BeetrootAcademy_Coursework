/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief Implement long and short relay work cycles using a single hardware counter
0. Use existing LED setup for control signal test 
 2. Upon timer interrupt:
  2.1 Reset the hardware time
  2.2 Update the hardware clock duration upon interrupt
  2.3 toggle the relay
3. Reassmeble in Relay schema 

 */

#include <Arduino.h>
// Вказівник на екземпляр апаратного таймера
hw_timer_t *timer = NULL;
constexpr uint8_t HW_TIMER_TICKS_DIVIDER = 80;
constexpr uint32_t COUNTER_DUTY_CYCLE = 1000000; // timer for relay NO mode; in microseconds
constexpr uint32_t COUNTER_IDLE = 4000000; // timer for relay NC mode; in microseconds
const int ALARM_LED_PIN = 4;   // Alternative PIN 
const int RELAY_PIN = 5; // Alarm activation switch 
// TODO: Change to static var once 
// Unneeded to use ENUM, since we only have states on/off 
// Доступна змінна з контексту переривання 
static volatile bool timerEventFlag = false;
//False - ввимкнене реле, True - ввімкнене реле


void blink(const int pin)   { static bool s; s = !s; digitalWrite(pin, s); }


void IRAM_ATTR onTimer() {
  // just create an event flag, no driver operation inside ISR
  timerEventFlag = true;
}


void setup() {
  Serial.begin(115200);

  pinMode(ALARM_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Номер таймера 0, подільник 80 (базова шина 80 МГц / 80 = 1 МГц, тобто 1 тік = 1 мкс),
  timer = timerBegin(0, HW_TIMER_TICKS_DIVIDER, true);

  // Прив'язка функції обробника; 
  timerAttachInterrupt(timer, &onTimer, true);

  // Уставка алярму в режимі off, увімкнено автоперезапуск (true)
  timerAlarmWrite(timer, COUNTER_IDLE, true);

  // Активація апаратного переривання таймера
  timerAlarmEnable(timer);

  //По черзі вимикаємо піни 
    const uint8_t pins[] = {ALARM_LED_PIN, RELAY_PIN};
    for (uint8_t pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

}

// TODO: 
// Захист від повторного запуску
// ЛЕД індикатори, що дублюють поточний стан
void loop() {
  static bool relayIsOn = false; 
  static uint32_t counter = 0;

  if (timerEventFlag) {
    timerEventFlag = false; // Clear the flag 
    relayIsOn = !relayIsOn; // Switch the timer
    if (relayIsOn){
      // Turn on the relay 
      timerAlarmWrite(timer, COUNTER_DUTY_CYCLE, true);  // 2. Write the duty cycle target value 
    }
    else{
      timerAlarmWrite(timer, COUNTER_IDLE, true);  // 2. Write the idle target value  
    }
    blink(RELAY_PIN);  
    // blink(ALARM_LED_PIN);  

    counter++;
    // int alarm_led_state = digitalRead(ALARM_LED_PIN);
    int relay_led_state = digitalRead(RELAY_PIN);
    Serial.printf("Time: %ld ms | Timer counter: %ld | LED ALARM  State %d \n", millis(), counter, relay_led_state);
}

}