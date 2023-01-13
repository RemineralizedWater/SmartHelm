// SOEN 422 Project: Helmet add-on for emergency workers
/*
 * The Coordinator file is for the TTGO connected to WiFi and our database.
 * It continuously monitors the database for changes, and sends those changes
 * to the TTGOs in the field using LoRa packets. The two thresholds that can
 * be modified through the database are the flame detection and the temperature
 * detection.
 * 
 * This code was developed by Simon Bilodeau, Amine Halimi, and Michael Rowe
 */
 
/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/ttgo-lora32-sx1276-arduino-ide/
*********/

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Libraries for WiFi Database:
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/RTDBHelper.h"

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Firebase stuff
#define API_KEY "AIzaSyC8uJT1hxp3ec2QxZncopVegIKUwlB7kyk"
#define DATABASE_URL "https://soen422-helmet-project-default-rtdb.firebaseio.com/"

//packet counter
int counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//WiFi stuff
char* ssid = "Pixel_6005";
char* password = "palomia69";

//Firebase variable
FirebaseData fb_data;
FirebaseAuth fb_auth;
FirebaseConfig fb_config; 
bool cnct_to_fb = false;

//Default values for thresholds
int temp_limit = 27;
int flame_limit = 1500;

/*
 * Connects the TTGO to wifi, then to the database. Initializes
 * the LoRa protocol, and tests the TTGO display.
 */
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  
  //WiFi Settings
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int timeout_counter = 0;
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(WiFi.status());
    Serial.print(", "); 
    timeout_counter++;
    //If the connection has no been found, then it restarts
    if(timeout_counter >= 50){
        ESP.restart();
    }
  }

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();
  
  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);


  //Firebase setup
  fb_config.api_key = API_KEY;
  fb_config.database_url = DATABASE_URL;

  if (Firebase.signUp(&fb_config, &fb_auth, "", ""))
  {
    Serial.println("Connected to the Database");
    cnct_to_fb = true;
  }
  else
  {

  }
  
  Firebase.begin(&fb_config, &fb_auth);
  Firebase.reconnectWiFi(true);
}

//Handles sending packets with LoRa
/*
 * Reads the values in the database for the temperature and 
 * flame thresholds, and sends a LoRa packet with the database
 * values to the HelmetUsers TTGO.
 */
void transferDataToLoRa()
{
  if (cnct_to_fb && Firebase.ready())
  {
    Serial.print("temp_limit:");
    Serial.print(temp_limit);
    Serial.print(" flame_limit:");
    Serial.print(flame_limit);
    Serial.print("\n");
    //Fetching temperature
    if (Firebase.RTDB.getInt(&fb_data, "/Thresholds/Temperature"))
    {
      temp_limit = fb_data.intData();
    }
    else
    {
      Serial.println("Using default or previously set value for temperature.");
    }

    if (Firebase.RTDB.getInt(&fb_data, "/Thresholds/Fire"))
    {
      flame_limit = fb_data.intData();
    }
    else
    {
      Serial.println("Using default or previously set value for Flame.");
    }
  }
  //Sending Packet with LoRa
  LoRa.beginPacket();
  LoRa.print(temp_limit);
  LoRa.print(";");
  LoRa.print(flame_limit);
  LoRa.endPacket();

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("LoRa packet sent.");
  display.setCursor(0,30);
  display.print(temp_limit);
  display.print(";");
  display.print(flame_limit);  
  display.display();
}

/*
 * calls transferDataToLoRa() every 1 seconds.
 */
void loop() {

  transferDataToLoRa();
  delay(1000);
}
