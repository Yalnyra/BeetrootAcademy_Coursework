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
const int ALARM_LED_PIN = 4;   // Alarm activation switch 
const int RELAY_PIN = 5; // Relay control pin 
// Змінна лічильника, доступна з контексту переривання
uint32_t last_count = 0; // Did counter update last? 
volatile uint32_t counter = 0;
// TODO: Change to static var once 
// Unneeded to use ENUM, since we only have states on/off 
volatile bool relayIsOn = false; 


void blinkAlarm()   { static bool s; s = !s; digitalWrite(ALARM_LED_PIN, s); }
void blinkRelay()   { static bool s; s = !s; digitalWrite(RELAY_PIN, s); }


void IRAM_ATTR onTimer() {
  timerAlarmDisable(timer);              // 1. Pause the alarm generation
  relayIsOn = !relayIsOn;
  counter++;
  if (relayIsOn){
    // Turn on the relay 
    timerAlarmWrite(timer, COUNTER_DUTY_CYCLE, true);  // 2. Write the duty cycle target value 
  }
  else{
    timerAlarmWrite(timer, COUNTER_IDLE, true);  // 2. Write the duty cycle target value  
  }
  // relayToggle();=
  blinkAlarm();  
  blinkRelay();
  timerWrite(timer, 0);                  // 3. Optional: Reset counter to 0 to avoid overshoot
  timerAlarmEnable(timer);               // 4. Re-enable the alarm
}

// (Unused) Ця функція автоматично викликається системою, якщо Watchdog таймер вийшов.
// extern "C" обов'язкове, щоб компілятор C++ правильно зв'язав цю функцію з ядром ESP-IDF.
// extern "C" void esp_task_wdt_isr_user_handler() {

//   // 1. Аварійно ВИМИКАЄМО небезпечне навантаження
//   digitalWrite(RELAY_PIN, LOW);

//   // 2. Вмикаємо візуальну індикацію аварії
//   digitalWrite(ALARM_LED_PIN, HIGH);

//   // 3. Збільшуємо лічильник аварій у енергонезалежній пам'яті
//   crash_counter++;

//   // 4. Оскільки ми встановили panic = false, система сама не перезавантажиться.
//   // Але залишати її завислою назавжди не можна, тому ми самі викликаємо ресет
//   // ПІСЛЯ того, як убезпечили обладнання.
//   esp_restart();
// }


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

  
  // Serial.printf("\n--- Запуск. Кількість попередніх аварій: %d ---\n", crash_counter);

  // // Ініціалізуємо Watchdog на 3 секунди, panic = FALSE
  // esp_task_wdt_init(3, false);
  // esp_task_wdt_add(NULL); 

  // Serial.printf("\n--- Запуск. Кількість попередніх аварій: %d ---\n", crash_counter);

  //По черзі вимикаємо піни 
    const uint8_t pins[] = {ALARM_LED_PIN, RELAY_PIN};
    for (uint8_t pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

}
// TODO: read the elapsed time during both (RelayOn/RelayOff) phases

// Опційно

// Watchdog
// Логування станів
// Захист від повторного запуску
// ЛЕД індикатори, що дублюють поточний стан
void loop() {
   
  if (counter != last_count ) {
    last_count = counter;
    int alarm_led_state = digitalRead(ALARM_LED_PIN);
    int relay_led_state = digitalRead(RELAY_PIN);
    Serial.printf("Time: %ld ms | Timer counter: %ld | LED ALARM  State %d", millis(), counter, alarm_led_state);
}

}