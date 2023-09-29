#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>
//#include <Adafruit_GFX.h>    // Core graphics library
//#include "Adafruit_ILI9341.h"
#include "makerfabs_pin.h"
#include <SPI.h>
#include <Wire.h>
#include "PCF8574.h"
#include <ArduinoWebsockets.h>

using namespace websockets;
WebsocketsServer server;
WebsocketsClient client;

PCF8574 pcf8574(PCF_ADD);

//Adafruit_ILI9341 tft = Adafruit_ILI9341(LCD_CS, LCD_DC, LCD_MOSI, LCD_SCK, LCD_RST);

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

const int w = 128;     // image width in pixels
const int h = 96;     // height

int xPos, yPos;                                // Abtastposition
int R_colour, G_colour, B_colour;              // RGB-Farbwert
int i, j;                                      // Zählvariable
float T_max, T_min;                            // maximale bzw. minimale gemessene Temperatur
float T_center;                                // Temperatur in der Bildschirmmitte


const char* ssid = "ESP32-THAT-PROJECT";
const char* password = "California";

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  //tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void setup() {
  Serial.begin(115200);
delay(1000);
  Serial.println("Camera Web Socket");

  //Wire.begin(ESP32_SDA, ESP32_SCL);

  //key_init();
    
//  if (! pcf8574.begin()) {
//    Serial.println("PCF8574 not found!");
//    while (1) delay(10);
// }

//  pinMode(LCD_CS, OUTPUT);
//  pinMode(LCD_DC, OUTPUT);
//  pinMode(LCD_BL, OUTPUT);
//  digitalWrite(LCD_BL, HIGH);
  
  tft.begin();
  tft.setRotation(1);

  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 319, 13, tft.color565(255, 0, 10));
  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
  
  Serial.println();
  Serial.println("Setting AP...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address : ");
  Serial.println(IP);

  server.listen(8888);
}

void loop() {
  if(server.poll()){
      client = server.accept();
    }

    if(client.available()){
      client.poll();

      WebsocketsMessage msg = client.readBlocking();

      uint32_t t = millis();

      // Get the width and height in pixels of the jpeg if you wish
      uint16_t w = 0, h = 0;
      TJpgDec.getJpgSize(&w, &h, (const uint8_t*)msg.c_str(), msg.length());
      Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);
    
      // Draw the image, top left at 0,0
      TJpgDec.drawJpg(0, 0, (const uint8_t*)msg.c_str(), msg.length());
    
      // How much time did rendering take (ESP8266 80MHz 271ms, 160MHz 157ms, ESP32 SPI 120ms, 8bit parallel 105ms
      t = millis() - t;
      Serial.print(t); Serial.println(" ms");
    }  

  //PCF8574::DigitalInput val = pcf8574.digitalReadAll();

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

int key_scanf()
{
    PCF8574::DigitalInput val = pcf8574.digitalReadAll();
    if (val.B_START == LOW)
        Serial.println("B_START PRESSED");
    if (val.B_SELECT == LOW)
        Serial.println("B_SELECT PRESSED");
    if (val.B_UP == LOW)
        Serial.println("B_UP PRESSED");
    if (val.B_DOWN == LOW)
        Serial.println("B_DOWN PRESSED");
    if (val.B_LEFT == LOW)
        Serial.println("B_LEFT PRESSED");
    if (val.B_RIGHT == LOW)
        Serial.println("B_RIGHT PRESSED");
    if (val.B_A == LOW)
        Serial.println("B_A PRESSED");
    if (val.B_B == LOW)
        Serial.println("B_B PRESSED");
    if (digitalRead(B_L) == LOW)
        Serial.println("B_L PRESSED");
    if (digitalRead(B_R) == LOW)
        Serial.println("B_R PRESSED");

    return 0;
}
