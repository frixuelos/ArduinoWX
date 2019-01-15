/***************************************************************************
  Arduino V1.1
  
  This is a small, cheap and easy weather station that sends data to APRS.fi
  It's based on BME280 library's examples and modified.
  
  Written by Alex (EA1IZV)
 ***************************************************************************/

#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// FOR BME/BMP280
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

// FOR WIFI CONNECTION
#define WSSID "" //NAME
#define PASSWD "" //PASSWORD
IPAddress staticIP(AAA,AAA,A,AAA);
IPAddress gateway(BBB,BBB,B,B);
IPAddress subnet(CCC,CCC,CCC,C);
IPAddress dns(8,8,8,8);

// FOR APRS
#define CALLSIGN ""
#define PASSCODE "" // GENERATE IT ON apps.magicbug.co.uk/passcode/
#define LATITUDE "" // LOOK !! -> 4 DIGITS POINT 2 DIGITS
#define LONGITUDE "" // LOOK !! -> 5 DIGITS POINT 2 DIGITS
#define SERVER_NAME "rotate.aprs.net"
#define PORT 14580

// FOR SAVE ENERGY
uint32_t SLEEP_TIME = 900e6; // BY DEFAULT EVERY 15 MINUTES

/*****************************************************
 *         SENDS APRS PACKET TO THE SERVER           *
 *****************************************************/
void sendAPRS(){
    WiFiClient client;
    int retr=20;
    while(!client.connect(SERVER_NAME, PORT) && retr>0){
      delay(50);
      --retr;
    }
    if(client.connected()){ // CONNECTED TO THE HOST
      Serial.println("APRS server connected !");
      /*********************************************** 
       *            APRS AUTHENTICATION              *
       ***********************************************/
      client.print("user ");
      client.print(CALLSIGN);
      client.print(" pass ");
      client.print(PASSCODE);
      client.print(" vers ArduinoWX 1.0 UDP 8888\n");
      /*********************************************** 
       *               SEND APRS DATA                *
       ***********************************************/
      Serial.println("Sending...");
      client.print(CALLSIGN);
      client.print(">APRS,TCPIP*:=");
      client.print(LATITUDE);
      client.print("/");
      client.print(LONGITUDE);
      client.print("_.../...g...t");
      client.print((int)(bme.readTemperature()*1.8+32));
      client.print("r...p...P...h");
      client.print((int)bme.readHumidity());
      client.print("b");
      client.print((int)(bme.readPressure()/10.0F));
      client.print("ArduinoWX\n");
     /*********************************************** 
      *              CLOSE CONNECTION               *
      ***********************************************/
      Serial.println("Done !");
      client.stop();
      
    }else{ // CONNECTION FAILURE
      Serial.println("Can't connect to the server (APRS)");
      client.stop();
    }
}

/*****************************************************
 *                   GOES TO SLEEP                   *
 ****************************************************/
void goSleep(){
    Serial.println("Sleeping zzZZZ");
    ESP.deepSleep(SLEEP_TIME);
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("ESP8266 + BME280 Weather station by EA1IZV"));

    /**************************************************
     *             WIFI CONNECTION BEGIN              *
     **************************************************/
    WiFi.hostname("ArduinoWX");
    WiFi.config(staticIP, gateway, subnet, dns);
    WiFi.mode(WIFI_STA); // ONLY CONECT TO WIFI ROUTER
    WiFi.begin(WSSID, PASSWD);
    
    Serial.print("Connecting");
    while(WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }

   Serial.println();
   Serial.print("Connected !\nIP address: ");
   Serial.println(WiFi.localIP());
   /**************************************************/   
   
    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        goSleep(); // NOT SEND APRS DATA JUST SAVE BATTERY
    }
    Serial.println("-- END OF BEGIN --");
    Serial.println();
    
    // SEND DATA
    sendAPRS();
    // DEEP SLEEP
    goSleep();
}

void loop() { 
  // DO NOTHING
}
