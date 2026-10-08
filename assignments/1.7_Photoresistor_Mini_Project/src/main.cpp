#include <Arduino.h>
#include <math.h> // Pow function 

// #define TELEPLOT_ON 

const int SENSOR_PIN = 6; // Пін підключення дільника з фоторезистором (GPIO In)
const int RELAY_PIN = 4;  // Пін керування транзистором реле (GPIO Out)

const int ADC_RESOLUTION = 12;
const int ADC_MAX = (int)round(pow(2, ADC_RESOLUTION) - 1);
// const int ADC_MAX = 4095 //default
const int THRESHOLD_DARK = 1800; 
const int THRESHOLD_LIGHT = 3000;
const float EMA_FILTER_ALPHA = 0.02;

/* Program state */
unsigned long previousMillis = 0;
const long interval = 500;

int prevOutValue = ADC_MAX / 2;

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW); 
  
  // встановлення роздільної здатності АЦП (12 біт = 0...4095)
  analogReadResolution(ADC_RESOLUTION);
}

/**
 @brief Exponential Moving Average. Instead of fixed window, averages between last value and filter output
 * Thus the bounce to outliers is never removed - only reduced in half by a baseline value
*/
float ema_next(float a, uint16_t x, uint16_t y){
    return int(a*x + (1-a)*y);
}


void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currentMillis = millis();

  int rstValue = analogRead(SENSOR_PIN);
  int outValue = ema_next(EMA_FILTER_ALPHA, rstValue, prevOutValue);
  // Store the output
  prevOutValue = outValue;
  
  if (rstValue > THRESHOLD_LIGHT){
    digitalWrite(RELAY_PIN, HIGH);
  }
  else if (rstValue <= THRESHOLD_DARK){
    digitalWrite(RELAY_PIN, LOW);
  }

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    
    int ctrl_input = digitalRead(RELAY_PIN);

    #ifdef TELEPLOT_ON
    Serial.printf(">Unfiltered Digital ADC:");
    Serial.println(rstValue);
    Serial.printf("> Filtered Digital ADC:");
    Serial.println(outValue);
    Serial.printf(">Trigger Output: ");
    Serial.println(ctrl_input);
    #else 
    // Serial.printf("Analog read: %d  |   calculated: %d  |   milli voltage: %d   |   diff: %.2f%% \n", rst, calculatedVoltage, voltValue, diff);
    Serial.printf("Timestamp: %d | Unfiltered Digital ADC: %d | Filtered Digital ADC: %d | Trigger Output:  %d | Trigger Floor: %d | Trigger Ceiling: %d\n", currentMillis, rstValue, outValue, ctrl_input, THRESHOLD_DARK, THRESHOLD_LIGHT);
    // Serial.printf("\n Timestamp: %d | ", currentMillis);
    // Serial.printf("\n Analogue read value: %d | ", rstValue);
    // Serial.printf("\n Digital Ctrt value: %d | ", ctrl_input);
    

    #endif
  }
}