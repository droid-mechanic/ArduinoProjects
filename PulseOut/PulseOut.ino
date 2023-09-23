void setup()
{
  pinMode(2, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
}

void loop()
{
  digitalWrite(2, HIGH);
  delayMicroseconds(100); // Approximately 10% duty cycle @ 1KHz
  digitalWrite(2, LOW);
  delayMicroseconds(1000 - 100);
}
