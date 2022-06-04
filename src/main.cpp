#include <Arduino.h>
#include <spi.h>
#include "FS.h"
// #include <SD.h>           // SD card reader library
// #include <SdFat.h>
#include  "iniSettings.h"     
#include <Wire.h>
// #include "sdspi.h"        // SD reader / writer functions and utilities
#include <RTClib.h>            // RTClib by Adafruit - Library for RTC-DS3231 real time clock
#include <ESP32Time.h> //https://forum.arduino.cc/t/esp32-how-to-add-timestamp-on-sd-files-withou-ntp/943194/15

ESP32Time Inrtc; // From library ESP32Time, allowing to set the internal ESP32 clock at the actual time
RTC_DS3231 RTC; // Real Time Clock object
IniSettings mySettings("/settings.ini"); // My settings object
/* #region SD card read and write declarations and functions */
    bool initSDCardReader()
      {
        if(!SD.begin(5))
          {
            Serial.println("Card Mount Failed");
            return false;
          }
        uint8_t cardType = SD.cardType();
        if(cardType == CARD_NONE)
          {
            Serial.println("No SD card attached");
            return false;
          }
        Serial.print("SD Card Type: ");
        if(cardType == CARD_MMC)
          {
            Serial.println("MMC");
          } else if(cardType == CARD_SD){
            Serial.println("SDSC");
          } else if(cardType == CARD_SDHC){
            Serial.println("SDHC");
          } else {
            Serial.println("UNKNOWN");
          }
        return true;
      }
/* #endregion */
void setup()
  {
    // put your setup code here, to run once:
    Serial.begin(115200);
    if (! RTC.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        abort();
      }
    DateTime now = RTC.now();
    Inrtc.setTime(now.unixtime()); // Internal ESP32 clock has been set at the actual time
    Serial.println("Initialisation de SD...");
    // SdFile::dateTimeCallback(dateTime);
    initSDCardReader(); // Preparation of SD card reader
    Serial.println("SD card initialized!");
    if(!mySettings.begin())
      {
        Serial.println("Error starting settings initialisation");
      }
    else
      {
        Serial.println("Settings initialisation OK!");
      }

    char buffer[BUFFER_LEN];
    char section[]="Section_2";
    char key[]="var";
    // mySettings.getValue(section, key, buffer);
    // Serial.printf("Valeur 1 = %s\n",mySettings.getvalue(section, key, buffer));
    Serial.printf("Valeur de [Wifi], sta_psw:%s\n",mySettings.getvalue("Wifi", "sta_psw", buffer));
    Serial.printf("Buffer:%s\n",buffer);
    Serial.println("___________________");
    // Serial.println("** 01 *****");mySettings.saveSettings("Wifi","sta_ip","19.19.19.19");
    // Serial.println("** 02 *****");mySettings.saveSettings("Wifi","sta_ip2","19.19.19.19");
    // Serial.println("** 03 *****");mySettings.saveSettings("Wifi1","sta_ip","192.192.192.192");
    // Serial.println("** 04 *****");mySettings.saveSettings("Section_2","var","newvalue");
    // Serial.println("** 05 *****");mySettings.saveSettings("Section_2","sta_ip","newKey");
  }

void loop() {
  // put your main code here, to run repeatedly:
}