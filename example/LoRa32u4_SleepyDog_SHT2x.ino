// Hello LoRa - ABP TTN Packet Sender (Multi-Channel)
// Tutorial Link: https://learn.adafruit.com/the-things-network-for-feather/using-a-feather-32u4
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Copyright 2015, 2016 Ideetron B.V.
//
// Modified by Brent Rubell for Adafruit Industries, 2018
//
// Sources:
// https://github.com/adafruit/TinyLoRa
// https://github.com/sabas1080/CayenneLPP
// https://github.com/adafruit/Adafruit_SleepyDog
// https://github.com/SodaqMoja/Sodaq_SHT2x

#include <SPI.h>
#include <Wire.h>
#include <TinyLoRa.h>
#include <CayenneLPP.h>
#include <Adafruit_SleepyDog.h>
#include <Sodaq_SHT2x.h>

/************************** Configuration ***********************************/

// Enable debug prints to serial monitor
// ! comment out this in production !
#define MY_DEBUG

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { SKey };

// Application Session Key (MSB)
uint8_t AppSkey[16] = { AppKey };

// Device Address (MSB)
uint8_t DevAddr[4] = { DevAddr };

// send lora every seconds (min. 60)
const unsigned SEND_INTERVAL = 600;

// sleep interval in seconds (min. 1 - max. 8)
const unsigned SLEEP_INTERVAL = 8;

// while sleep counter
unsigned int counter = 0;

// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8);

// LoRa Cayenn LPP
CayenneLPP lpp(51);

/************************** End configuration ***********************************/

void setup()
{
    delay(5000);

    #ifdef MY_DEBUG
    Serial.begin(9600);
    while(!Serial);
    Serial.println(F("Starting"));
    #endif
    
    // start wire for SHT2x
    Wire.begin();

    // Initialize pin LED_BUILTIN as an output
    pinMode(LED_BUILTIN, OUTPUT);

    // define multi-channel sending
    lora.setChannel(MULTI);

    // set datarate
    lora.setDatarate(SF7BW125);

    // if error init stop cycle
    if(!lora.begin()) {
        #ifdef MY_DEBUG
        Serial.println("Failed");
        Serial.println("Check your radio");
        #endif
        while(true) {
            // blink LED to indicate an error
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
        }
    }
}


void loop()
{
    getData();
    lora.sendData(lpp.getBuffer(), lpp.getSize(), lora.frameCounter);
    lora.frameCounter++;

    // sleep CPU
    counter = 0;
    while(counter < SEND_INTERVAL/SLEEP_INTERVAL) {
        counter++;
        #ifdef MY_DEBUG
        Serial.print(F("Sleep counter: "));
        Serial.println(counter);
        // emulate sleep cpu (USB serial is disconected when you use Watchdog.sleep)
        delay(SLEEP_INTERVAL*1000);
        #else
        Watchdog.sleep(SLEEP_INTERVAL*1000);
        #endif
    }
}


void getData()
{
    lpp.reset();
    lpp.addTemperature(5, SHT2x.GetTemperature());
    lpp.addRelativeHumidity(6, SHT2x.GetHumidity());
    
    #ifdef MY_DEBUG
    Serial.print(F("Humidity(%RH): "));
    Serial.println(SHT2x.GetHumidity());
    Serial.print(F("Temperature(C): "));
    Serial.println(SHT2x.GetTemperature());
    #endif

}
