#include <Servo.h>

int sensorPin = A0;
int sensorValue = 0;

Servo pump;

const int onPumpValue = 25;
const int offPumpValue = 90;

int currentPumpValue = 90;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pump.attach(2);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);

  if (sensorValue >= 600)
  {
    currentPumpValue = onPumpValue;
  }

  if (sensorValue < 500)
  {
    currentPumpValue = offPumpValue;
  }
  
  pump.write(currentPumpValue);
}
