#include <Arduino.h>

const int CONTROL_PIN = 15;

void turnOnRed()
{
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);
}

void turnOnBlue()
{
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);
}

void turnOffBoth()
{
  // High-Z
  pinMode(CONTROL_PIN, INPUT);
}

void setup()
{
  turnOffBoth();
}

void loop()
{
  int flashSpeed = 60;

  for (int i = 0; i < 3; i++)
  {
    turnOnRed();
    delay(flashSpeed);
    turnOffBoth();
    delay(flashSpeed);
  }

  delay(1500);

  for (int i = 0; i < 3; i++)
  {
    turnOnBlue();
    delay(flashSpeed);
    turnOffBoth();
    delay(flashSpeed);
  }

  delay(400);
}