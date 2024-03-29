  /*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
  This example shows how to send HID (keyboard/mouse/etc) data via BLE
  Note that not all devices support BLE keyboard! BLE Keyboard != Bluetooth Keyboard
*/

#include <Arduino.h>
#include <SPI.h>

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"
#include "keycode.h"

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
/*=========================================================================*/


// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

typedef struct
{
  uint8_t modifier;   /**< Keyboard modifier keys  */
  uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
  uint8_t keycode[10]; /**< Key codes of the currently pressed keys. */
} hid_keyboard_report_t;

// Report that send to Central every scanning period
hid_keyboard_report_t keyReport = { 0, 0, { 0 } };

// Report sent previously. This is used to prevent sending the same report over time.
// Notes: HID Central intepretes no new report as no changes, which is the same as
// sending very same report multiple times. This will help to reduce traffic especially
// when most of the time there is no keys pressed.
// - Init to different with keyReport
hid_keyboard_report_t previousReport = { 0, 0, { 1 } };

int joyPin1 = 18;
int joyPin2 = 19;
int mouseClickPinR = 5;
int mouseClickPinL = 20;

int keys = 8; //Number of keyboard keys (Does not include mouse buttons)

int mouseClickMemR = 0;
int mouseClickMemL = 0;
int mouseClickR = 0;
int mouseClickL = 0;

int joyReadCounter = 1;
int joyGain = 1;
int joyDeadZone = 10;

int joyAV1 = 512;
int joyAV2 = 512;

int joyMidVal = 512;

int joyMoveVal1 = 0;
int joyMoveVal2 = 0;

// GPIO corresponding to HID keycode (not Ancii Character)
int inputPins[10]     = { 22        , 20        , 6        , 11       , 13       , 21        , 2       , 3       , 18       , 19        };
int inputKeycodes[10] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5 , HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9 };

int inputPinsOn[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int inputPinsMem[10]={inputPinsOn};

int j;





/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit HID Keyboard Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    ble.factoryReset();
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Enable HID Service if not enabled */
  int32_t hid_en = 0;
  
  ble.sendCommandWithIntReply( F("AT+BleHIDEn"), &hid_en);

  if ( !hid_en )
  {
    Serial.println(F("Enable HID Service (including Keyboard): "));
    ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ));

    /* Add or remove service requires a reset */
    Serial.println(F("Performing a SW reset (service changes require a reset): "));
    !ble.reset();
  }
  
  Serial.println();
  Serial.println(F("Go to your phone's Bluetooth settings to pair your device"));
  Serial.println(F("then open an application that accepts keyboard input"));
  Serial.println();

  // Set up input Pins
  for(int i=0; i< 10; i++)
  {
    pinMode(inputPins[i], INPUT_PULLUP);
  }
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  /* scan GPIO, since each report can have up to 6 keys
   * we can just assign a slot in the report for each GPIO 
   * 
   * Can't do that anymore, since have more than 6 keys
   * need to assign keys to slots temporarily depending on which one is pushed
   * Also need to ensure that the released key is sent in that slot when a key is relased
   * 
   */
  if ( ble.isConnected() )
  {
      
    // Loop through pins and get readouts ------------------------------------
    for(int i=0; i<10; i++)
    {
      if ( digitalRead(inputPins[i]) == LOW )
      {
        inputPinsOn[i]= 1;
      }else
      {
        inputPinsOn[i]= 0;
      }
    }
      // Put the keycodes in the command sequence if they exist, or if they existed in the previous set-------------------------------------
      j = 0;
      for (int i=0; i<10; i++)
      {
        if ( j < 6  )
        {
          if ( inputPinsOn[i] == 1 )
          {
            keyReport.keycode[j] = inputKeycodes[i];
            j=j+1;
          }
        }
      }
      
      // Add extra zeros to get to 6 codes
      // Probably not needed, but 
      while (j < 6)
      {
        keyReport.keycode[j] = 0;
        j=j+1;
      }

/*      
      // GPIO is active low     
      if ( digitalRead(inputPins[i]) == LOW )
      {
        keyReport.keycode[i] = inputKeycodes[i];
      }else
      {
        keyReport.keycode[i] = 0;
      }
*/
      
      // Only send if it is not the same as previous report ---------------------------
      if ( memcmp(&previousReport, &keyReport, 6) )
      {
        // Send keyboard report
        ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 6);
Serial.println("Report Sent-----------------------------------------------------");
        
        // copy to previousReport
        memcpy(&previousReport, &keyReport, 6);
       }
  }else{
Serial.println("Not Connected");
  }

  // scaning period is 10 ms
  delay(10);
}
