/**
 * @author Yevhenii Vinokur @ 2026
 * @file    main.cpp
 * @brief Rewrite debouncing examples in ESP-IDF code using 4 solutions:
 * 1. Boilerplate: interrupt w/o debounce
 * 2. Ignore interrupts for a period after the button is pressed; release still pass-through 
*  3. ISR = Event; Loop only sets button to ON if button is still in HIGH state; otherwise button is OFF '
*  4. Full FSM with a delay between IDLE -> DEBOUNCE_PRESSED -> IGNORE_TRANSITIONS
*  5. 1 with Hardware pull-up resistor + capacitor sequence 
* CODE FOR 2., 3. BROKEN SINCE COMMIT 
    Method        | Additional counts | Delay                                | Complexity | Comment
    No Debounce   | 1-4               | 0                                    | Easy.      | This is easy implementing but there are additional triggers
    With Debounce | interaction ended | ~50ms                                | Easy.      | Is easy implementing and more accurate than previous method
    State Based.  | 0                 | 0                                    | Medium.    | Totally correct count but small delay exists
    Polling FSM   | 0                 | ~50ms                                | Medium.    | No ISR, 4-state FSM debounces press & release, deterministic timing
    Hardware RC   | 1(0 for 3 and 4). | None. (just when capacitor is empty) | Hard.      | It works with perfect accuracy, but additional hardware is needed
*

 */

#include <Arduino.h>

// --- Old (interrupt + time-based debounce) — kept for reference, to be removed ---

const unsigned int BUTTON_PIN = 1;
const unsigned long DEBOUNCE_MS = 50;
const unsigned long POLL_MS = 50;
// Static state variables 
enum class BtnState { IDLE, DEBOUNCE_PRESS, HELD, DEBOUNCE_RELEASE };
BtnState btnState = BtnState::IDLE;
volatile bool isrTriggered = false;
unsigned long previousPushTime = 0;
unsigned long buttonPressedMillis = 0;

unsigned int counter = 0;
unsigned int last_count = 0;
unsigned long isrMillis = 0;

void IRAM_ATTR buttonISR() {
    isrTriggered = true;
    isrMillis = millis();
}

/* Case 1 & 5 
* @brief Interrupt call means button is pressed
*/
void checkButtonState_Passthrough(){
    if (isrTriggered){
        counter++;
        isrTriggered = false;
    }
}


/* Case 2
* @brief 2. Ignore interrupts for a period after the button is pressed; release still pass-through 
* @return Button is pressed or not 
*/
void checkButtonState_Delay() {
    if(isrTriggered && previousPushTime + DEBOUNCE_MS < millis()) {
        previousPushTime = millis();
        isrTriggered = false;

        counter++;
        // if (digitalRead(BUTTON_PIN) == LOW && btnState != BtnState::HELD) {
        //     btnState = BtnState::HELD;
        //     counter++;
        // } else if (digitalRead(BUTTON_PIN) == HIGH) {
        //     btnState = BtnState::IDLE;
        // }
        Serial.println(millis() - isrMillis);
    }
}


/* Case 3
* @brief 3.  Debounce through logic level check upon press event 
* ISR = Event; Loop only sets button to ON if button is still in HIGH state; otherwise button is OFF '
*/
void checkButtonState_Interrupt() {
    if (isrTriggered) {
        isrTriggered = false;
        
        // Original implementation
        // if (nextState == BtnState::HELD && btnState != BtnState::HELD) {
        //     counter++;
        // }
        // // Accept if button state has changed
        if (digitalRead(BUTTON_PIN) == LOW) {
            if (btnState == BtnState::IDLE || btnState == BtnState::DEBOUNCE_RELEASE) {
                btnState = BtnState::HELD;
                counter++;
            }
        } else {
            // Ігноруємо відпускання
            btnState = BtnState::IDLE;
        }
    }
}

// Polling vs interrupt based state control 
//
// In setup(): attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
// --------------------------------------------------------------------------------

// Task 4: Polling + debounce as a state machine (no interrupts)
// Poll the pin every POLL_MS; a 4-state FSM filters bouncing on both press and release.

unsigned long stateEnteredAt = 0;
unsigned long lastPollAt = 0;

// Task 4: Polling + debounce (without interrupts)
void pollButton() {
    unsigned long now = millis();
    if (now - lastPollAt < POLL_MS) return;
    lastPollAt = now;

    bool pressed = (digitalRead(BUTTON_PIN) == LOW);

    switch (btnState) {
        case BtnState::IDLE:
            if (pressed) {
                btnState = BtnState::DEBOUNCE_PRESS;
                stateEnteredAt = now;
            }
            break;

        case BtnState::DEBOUNCE_PRESS:
            if (!pressed) {
                btnState = BtnState::IDLE;
            } else if (now - stateEnteredAt >= DEBOUNCE_MS) {
                counter++;
                btnState = BtnState::HELD;
            }
            break;

        case BtnState::HELD:
            if (!pressed) {
                btnState = BtnState::DEBOUNCE_RELEASE;
                stateEnteredAt = now;
            }
            break;

        case BtnState::DEBOUNCE_RELEASE:
            if (pressed) {
                btnState = BtnState::HELD;
            } else if (now - stateEnteredAt >= DEBOUNCE_MS) {
                btnState = BtnState::IDLE;
            }
            break;
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // CHANGE використовується для того, щоб відслідковувати як натискання, так і відпускання кнопки 
    // для правильної роботи BtnState::IDLE у методах 2 та 3
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
    Serial.print("Interrupt state: ", isrTriggered)
    // buttonStateSubscriber();
    // Залиште розкоментованою лише ОДНУ реалізацію
    checkButtonState_Passthrough(); // Завдання 1 і 5 
    // checkButtonState_Delay();      // Завдання 2
    // checkButtonState_Interrupt();  // Завдання 3
    // pollButton();                     // Завдання 4
    unsigned long currentMillis = millis();
    
    if (counter != last_count) {
        last_count = counter;
        Serial.printf("Button Pressed! Count: %u | Interrupt latency: %lu ms\n", last_count, currentMillis - isrMillis);
    }
}