#include <Arduino.h>
const int SENSOR_PIN = 6; // Пін підключення дільника з фоторезистором (GPIO In)
const int RELAY_PIN = 4;  // Пін керування транзистором реле (GPIO Out)

const int ADC_RESOLUTION = 12
const int THRESHOLD_DARK = 1000; 
const int THRESHOLD_LIGHT = 2500;

/* Program state */
unsigned long previousMillis = 0;
const long interval = 50;

uint16_t previousOut = 4095 / 2 

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW); 
  
  // встановлення роздільної здатності АЦП (12 біт = 0...4095)
  analogReadResolution(12);
}

/**
 @brief 
*/
float ema_next(float a, uint16_t x, uint16_t y){
    return int(a*x + (1-a)*y);
}


void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currentMillis = millis();

  int rstValue = analogRead(SENSOR_PIN);
  
  if (rstValue > THRESHOLD_LIGHT){
    digitalWrite(RELAY_PIN, LOW);
  }
  else if (rstValue <= THRESHOLD_DARK){
    digitalWrite(RELAY_PIN, HIGH);
  }

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    
    int ctrl_input = digitalRead(RELAY_PIN);

    // Check button press & LED state for Debugging 
    // Serial.printf("\n Timestamp: %d | ", currentMillis);
    // Serial.printf("\n Analogue read value: %d | ", rstValue);
    // Serial.printf("\n Digital Ctrt value: %d | ", ctrl_input);
    
    // Serial.print("\n Timestamp: %d | ", currentMillis);
    Serial.printf(">Raw:");
    Serial.println(rstValue);
    // Serial.printf("\n Digital Ctrt value: %d | ", ctrl_input);
    
  }
}