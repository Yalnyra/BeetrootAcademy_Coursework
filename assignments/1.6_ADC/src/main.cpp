/** @author Yevhenii Vinokur @ 2026
 @brief Measure the real (Analog) vs Digital Voltage and compute a precision error
 * Output files: output.log - Precision error for each ADC analogRead() value,
 * Table struct Elaborated at line 19 
 * sorted_output.log - Same log, but sorted by Digital ADC value descending
 * adc_error_vs_voltage.png - Precision error (Y) plotted against Analog ADC voltage (X)
 * Although the data for Voltage above 600 mV is lacking in the plot, 
 * Mean Error is climbing as Voltage drops from 500mV (Dark) to 300mV (Almost full blackness)
 * From an Average of 20% to an Average of 30%
 */
#include <Arduino.h>
#include <math.h> // Pow function 
#include <map> // Unique <int:float> dictionary
#include <vector> // For recording the table 
const int SENSOR_PIN = 7; // GPIO input connector pin of photoresistor connected to ADC 

const int ADC_RESOLUTION = 12;
// const int ADC_MAX = (int)round(pow(2, ADC_RESOLUTION) - 1);
int ADC_MAX = 4095;
const float U_REF = 3100.;
/* Program state */
unsigned long previousMillis = 0;
const long interval = 200;
/* Implements following structure 
* Key (int) rstDigitalValue - ADC analogRead() value 
* Value - Vector of [rstAnalogValue, rstAnalogValueAnalytic, rstError]:
  * rstAnalogValue - read using analogReadMilliVolts()
  * rstAnalogValueAnalytic - Analytic using formula (ADC_MAX / analogRead) * MAX_VLT
  * rstError - precision error of real analog vs analytic analog value  
*/
std::map<int,std::vector<float>> rstValuesTable; 
void setup() {
  Serial.begin(115200);
  rstValuesTable = {};
  
  // Not needed under analog input 
  // pinMode(SENSOR_PIN, INPUT);
  // встановлення роздільної здатності АЦП (12 біт = 0...4095)
  analogReadResolution(ADC_RESOLUTION);


}


void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currentMillis = millis();

  // Read Photoresisor values 
  int  rstDigitalValue = analogRead(SENSOR_PIN);
  float rstAnalogValue = (float)analogReadMilliVolts(SENSOR_PIN);
  
  // Calculate precision error 
  // Convert to milliVolts first 
  float rstAnalogValueAnalytic = (rstDigitalValue / (float)ADC_MAX) * U_REF;
  float rstError = abs(rstAnalogValue - rstAnalogValueAnalytic)/ rstAnalogValue * 100.;

  // TODO Save value to a table only if change was unique 

  if (!rstValuesTable.count(rstDigitalValue)){
    rstValuesTable[rstDigitalValue] = {rstAnalogValue, rstAnalogValueAnalytic, rstError};

    Serial.printf("\nDigital ADC : %d | ", rstDigitalValue);
    Serial.printf("Analogue ADC: %f | ", rstAnalogValue);
    Serial.printf("Analogue Analytic ADC: %f | ", rstAnalogValueAnalytic);
    Serial.printf("Precision Error: %f | ", rstError);
    Serial.printf("Timestamp: %d", currentMillis);
  }
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    
    // Generally check
    // Serial.printf("\n | Digital ADC : %d | ", rstDigitalValue);
    // Serial.printf("Analogue ADC: %f | ", rstAnalogValue);
    // Serial.printf("Analogue Analytic ADC: %f | ", rstAnalogValueAnalytic);
    // Serial.printf("Precision Error: %f | ", rstError);
    // Serial.printf("| Timestamp: %d | ", currentMillis);
  }
}