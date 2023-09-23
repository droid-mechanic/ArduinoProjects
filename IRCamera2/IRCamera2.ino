#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Adafruit_MLX90640.h>

#define TFT_CS        12
#define TFT_RST       13
#define TFT_DC        14
#define TFT_MOSI      15  // Data out
#define TFT_SCLK      16  // Clock out

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

int xPos, yPos;                                // Abtastposition
int R_colour, G_colour, B_colour;              // RGB-Farbwert
int i, j;                                      // Zählvariable
float T_max, T_min;                            // maximale bzw. minimale gemessene Temperatur
float T_center;                                // Temperatur in der Bildschirmmitte

void setup() {
  Serial.begin(9600);

  Serial.println("Adafruit MLX90640 Simple Test");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_2_HZ);
  
  tft.init(135, 240);           // Init ST7789 240x135
  tft.setRotation(3);

  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, 0, 239, 13, tft.color565(255, 0, 10));
  tft.setCursor(25, 3);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW, tft.color565(255, 0, 10));
  tft.print("Thermographie");    

  tft.drawLine(220, 120 - 0, 239, 120 - 0, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 30, 239, 120 - 30, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 60, 239, 120 - 60, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 90, 239, 120 - 90, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 120, 239, 120 - 120, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 150, 239, 120 - 150, tft.color565(255, 255, 255));
  tft.drawLine(220, 120 - 180, 239, 120 - 180, tft.color565(255, 255, 255));

  tft.setCursor(40, 110);
  tft.setTextColor(ST77XX_WHITE, tft.color565(0, 0, 0));
  tft.print("T+ = ");
// drawing the colour-scale
    // ========================
 
    for (i = 0; i < 181; i++)
       {
        //value = random(180);
        
        getColour(i);
        tft.drawLine(220, 120 - i, 239, 120 - i, tft.color565(R_colour, G_colour, B_colour));
       } 
}

void loop() {
  delay(500);
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }

  frame[1*32 + 21] = 0.5 * (frame[1*32 + 20] + frame[1*32 + 22]);    // eliminate the error-pixels
    frame[4*32 + 30] = 0.5 * (frame[4*32 + 29] + frame[4*32 + 31]);    // eliminate the error-pixels
    
    T_min = frame[0];
    T_max = frame[0];

    for (i = 1; i < 768; i++)
       {
        if((frame[i] > -41) && (frame[i] < 301))
           {
            if(frame[i] < T_min)
               {
                T_min = frame[i];
               }

            if(frame[i] > T_max)
               {
                T_max = frame[i];
               }
           }
        else if(i > 0)   // temperature out of range
           {
            frame[i] = frame[i-1];
           }
        else
           {
            frame[i] = frame[i+1];
           }
       }

  T_center = frame[11* 32 + 15];    

    // drawing the picture
    // ===================

    for (i = 0 ; i < 24 ; i++)
       {
        for (j = 0; j < 32; j++)
           {
            frame[i*32 + j] = 180.0 * (frame[i*32 + j] - T_min) / (T_max - T_min);
                       
            getColour(frame[i*32 + j]);
            
            tft.fillRect(217 - (32 - j) * 5, 35 + (24 - i) * 5, 5, 5, tft.color565(R_colour, G_colour, B_colour));
           }
       }
    
    //tft.drawLine(217 - 15*7 + 3.5 - 5, 11*7 + 35 + 3.5, 217 - 15*7 + 3.5 + 5, 11*7 + 35 + 3.5, tft.color565(255, 255, 255));
    //tft.drawLine(217 - 15*7 + 3.5, 11*7 + 35 + 3.5 - 5, 217 - 15*7 + 3.5, 11*7 + 35 + 3.5 + 5,  tft.color565(255, 255, 255));
 
    //tft.fillRect(260, 25, 37, 10, tft.color565(0, 0, 0));
    //tft.fillRect(260, 205, 37, 10, tft.color565(0, 0, 0));    
    //tft.fillRect(115, 220, 37, 10, tft.color565(0, 0, 0));    

    tft.setTextColor(ST77XX_WHITE, tft.color565(0, 0, 0));
    tft.setCursor(220, 25);
    tft.print(T_max, 1);
    tft.setCursor(220, 120);
    tft.print(T_min, 1);
    tft.setCursor(120, 120);
    tft.print(T_center, 1);

    //tft.setCursor(300, 25);
    //tft.print("C");
    //tft.setCursor(300, 205);
    //tft.print("C");
    //tft.setCursor(155, 220);
    //tft.print("C");

}

// ===============================
// ===== determine the colour ====
// ===============================

void getColour(float j)
   {
    if (j >= 0 && j < 30)
       {
        R_colour = 0;
        G_colour = 0;
        B_colour = 20 + (120.0/30.0) * j;
       }
    
    if (j >= 30 && j < 60)
       {
        R_colour = (120.0 / 30) * (j - 30.0);
        G_colour = 0;
        B_colour = 140 - (60.0/30.0) * (j - 30.0);
       }

    if (j >= 60 && j < 90)
       {
        R_colour = 120 + (135.0/30.0) * (j - 60.0);
        G_colour = 0;
        B_colour = 80 - (70.0/30.0) * (j - 60.0);
       }

    if (j >= 90 && j < 120)
       {
        R_colour = 255;
        G_colour = 0 + (60.0/30.0) * (j - 90.0);
        B_colour = 10 - (10.0/30.0) * (j - 90.0);
       }

    if (j >= 120 && j < 150)
       {
        R_colour = 255;
        G_colour = 60 + (175.0/30.0) * (j - 120.0);
        B_colour = 0;
       }

  if (j >= 150 && j <= 180)
  {
     R_colour = 255;
     G_colour = 235 + (20.0/30.0) * (j - 150.0);
     B_colour = 0 + 255.0/30.0 * (j - 150.0);
  }
}
