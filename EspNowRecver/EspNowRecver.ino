#include "WiFi.h"
#include "esp_now.h"
#include <ESP32Servo.h> 

#define bucketServoPin  23

#define auxServoPin 22

Servo bucketServo;
Servo auxServo;
struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {32, 33},  //RIGHT_MOTOR Pins (IN1, IN2)
  {5, 18},  //LEFT_MOTOR  Pins
  {19, 21}, //ARM_MOTOR pins 
};

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define ARMUP 5
#define ARMDOWN 6
#define STOP 0


#define RIGHT_MOTOR 1
#define LEFT_MOTOR 0
#define ARM_MOTOR 2

#define FORWARD 1
#define BACKWARD -1

bool removeArmMomentum = false;
int bucketPos = 90;
const int bucketMin = 10;
const int bucketMax = 180;

typedef struct struct_message {
  char d;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Char received: ");
  Serial.println(myData.d);
  if (myData.d == 'S') {
     //?
    } else if (myData.d == 'U') {
      moveCar(UP);
    } else if (myData.d == 'D') {
      moveCar(DOWN);
    } else if (myData.d == 'L') {
      moveCar(LEFT);
    } else if (myData.d == 'R') {
      moveCar(RIGHT);
    } else if (myData.d == 'A') {
      moveCar(ARMUP);
    } else if (myData.d == 'B') {
      moveCar(ARMDOWN);
    } else if (myData.d == 'C') {
      moveCar(STOP);
      moveBucketDown();
    } else if (myData.d == 'E') {
      moveCar(STOP);
      moveBucketUp();
    } else {
      moveCar(STOP);
    }
}

void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    if(removeArmMomentum)
    {
    digitalWrite(motorPins[ARM_MOTOR].pinIN1, HIGH);
    digitalWrite(motorPins[ARM_MOTOR].pinIN2, LOW); 
    delay(5);
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
    delay(5);
    digitalWrite(motorPins[ARM_MOTOR].pinIN1, HIGH);
    digitalWrite(motorPins[ARM_MOTOR].pinIN2, LOW);
    delay(10);  
    removeArmMomentum = false;
    }
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue); 
  switch(inputValue)
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                  
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD); 
      break;
 
    case STOP:
      rotateMotor(ARM_MOTOR, STOP); 
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;

    case ARMUP:
      rotateMotor(ARM_MOTOR, FORWARD);
      break;
      
    case ARMDOWN:
      rotateMotor(ARM_MOTOR, BACKWARD);
      removeArmMomentum = true;
      break; 
      
    default:
      rotateMotor(ARM_MOTOR, STOP);    
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP); 
      break;
  }
}

void moveBucketUp() {
  if (bucketPos <= bucketMax)
  {
    bucketPos += 5;
  }
  bucketTilt(bucketPos);
}

void moveBucketDown() {
  if (bucketPos >= bucketMin)
  {
    bucketPos -= 5;
  }
  bucketTilt(bucketPos);
}

void bucketTilt(int bucketServoValue)
{
  bucketServo.write(bucketServoValue); 
}
void auxControl(int auxServoValue)
{
  auxServo.write(auxServoValue); 
}

void setUpPinModes()
{
      
  for (int i = 0; i < motorPins.size(); i++)
  {   
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  
  }
  moveCar(STOP);
  bucketServo.attach(bucketServoPin);
  auxServo.attach(auxServoPin);
  auxControl(150);
  bucketTilt(150);
}

void setup() {
  setUpPinModes();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initiailizing esp now");
    return;
  }

  bucketTilt(bucketPos);

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // put your main code here, to run repeatedly:

}
