#include "WiFi.h"
#include "esp_now.h"
#include "makerfabs_pin.h"
#include "PCF8574.h"
#include <Wire.h>

PCF8574 pcf8574(PCF_ADD);
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0x17, 0x47, 0xFC};

typedef struct struct_message {
  char d;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Successful" : "Failed");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  Wire.begin(ESP32_SDA, ESP32_SCL);
  
  key_init();
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initiailizing esp now");
    return;
  }

  if (! pcf8574.begin()) {
    Serial.println("PCF8574 not found!");
    while (1) delay(10);
 }
 
  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("failed to add peer");
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  PCF8574::DigitalInput val = pcf8574.digitalReadAll();
    if (val.B_START == LOW) {
      myData.d = 'S';
    } else if (val.B_UP == LOW) {
      myData.d = 'U';
    } else if (val.B_DOWN == LOW) {
      myData.d = 'D';
    } else if (val.B_LEFT == LOW) {
      myData.d = 'L';
    } else if (val.B_RIGHT == LOW) {
      myData.d = 'R';
    } else if (val.B_A == LOW) {
      myData.d = 'A';
    } else if (val.B_B == LOW) {
      myData.d = 'B';
    } else if (digitalRead(B_L) == LOW) {
      myData.d = 'C';
    } else if (digitalRead(B_R) == LOW) {
      myData.d = 'E';
    } else {
      myData.d = 'N';
    }

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent Successfully");
  } else {
    Serial.println("Error sending!");
  }
  delay(100);
}


void key_init()
{
    for (int i = 0; i < 8; i++)
    {
        pcf8574.pinMode(i, INPUT);
    }
    pinMode(B_L, INPUT);
    pinMode(B_R, INPUT);
}
