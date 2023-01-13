// SOEN 422 Project: Helmet add-on for emergency workers
/*
 * The HelmetUsers file is for the TTGO connected to the sensors. 
 * It includes code for a temperature sensor, a flame sensor, and an IR sensor.
 * The temperature sensor activates a servo simulating a fan when its threshold
 * is hit.
 * The flame sensor makes a specific buzzer sound when a flame is detected.
 * The IR sensor makes a specific buzzer sound when an object is detected.
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
#define BAND 915E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;

// Temp sensor things
#include "DHT.h"
#include <ESP32Servo.h>

#define DHTPIN 21

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
int pos = 0;
Servo servo_16;

#define RX 17
#define TX 25

// Flame sensor things
int flame_Sensor_Buzzer_Pin = 13;
int flame_Sensor_Analog_Pin = 38;
int flame_Sensor_Digital_Pin = 39;

// IR sensor things
int sensorBuzzerPin = 13;
int irSensorDigitalPin = 12;

// LoRa packet values
int temp_limit = 25;
int flame_limit = 1000;

/*
 * Sets up TTGO display, and initializes LoRa protocol.
 */
void setup() { 
  //initialize Serial Monitor
  Serial.begin(115200);
  
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
  display.print("LORA RECEIVER ");
  display.display();

  Serial.println("LoRa Receiver Test");
  
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
  display.println("LoRa Initializing OK!");
  display.display();  

  // temp sensor setup
  dht.begin();
  servo_16.attach(16, 500, 2500);
  Serial1.begin(9600, SERIAL_8N1, RX, TX);

  // flame sensor setup
  pinMode(flame_Sensor_Buzzer_Pin, OUTPUT);
  pinMode(flame_Sensor_Digital_Pin, INPUT);

  // IR sensor setup
  pinMode(sensorBuzzerPin, OUTPUT);
  pinMode(irSensorDigitalPin, INPUT);

}

// temp sensor method
/*
 * Checks multiple temperature values, uses the heat index
 * value to determine if the servo (fan) must spin.
 */
void tempDetection() {
  // temp sensor looping
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t)|| isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("\nHumidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
//  Serial.print(f);
//  Serial.print(F("°F  Heat index: "));
  Serial.print(F(" Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
//  Serial.print(hif);
//  Serial.println(F("°F"));

  if (hic > temp_limit) {
    Serial1.print("1");
  } else {
    Serial1.print("0");
  }
  delay(50);
}

// flame sensor method
/*
 * Checks multiple flame values. Uses the analog value to determine if
 * the buzzer must sound.
 */
void flameDetection()
{
  int flame_Sensor_Analog_Value = analogRead(flame_Sensor_Analog_Pin);
  int flame_Sensor_Digital_Value = digitalRead(flame_Sensor_Digital_Pin);

  Serial.print("\nFlame Sensor: ");
  Serial.print(flame_Sensor_Analog_Value);
  Serial.print("\t");
  Serial.print("Flame Class: ");
  Serial.print(flame_Sensor_Digital_Value);
  Serial.print("\t");

  //if (flame_Sensor_Analog_Value < flame_limit && flame_Sensor_Digital_Value == 1)
  if (flame_Sensor_Analog_Value < flame_limit)
  {
    Serial.println("Flame Detected");
    tone(flame_Sensor_Buzzer_Pin, 738, 0.9 * 1000);
    delay(0.1 * 1000);
    noTone(flame_Sensor_Buzzer_Pin);
  }
  else
  {
    Serial.print("FlameUndetected");
  }
  delay(50);
}

// IR sensor method
/*
 * Checks if an object is in the direction that the IR sensor is pointing
 * and makes a sound through the buzzer if so.
 */
void irDetection()
{
  int irSensorDigitalValue = digitalRead(irSensorDigitalPin);

  Serial.print("\nIR Sensor: ");
  Serial.print("\t");
  Serial.print("Person Detected (1=yes, 0=no)?: ");
  Serial.print(irSensorDigitalValue);
  Serial.print("\t");

  if (irSensorDigitalValue == 1) {
    Serial.println("Object Detected");
    tone(sensorBuzzerPin, 184.5, 0.9 * 1000); // Frequency to key: F#3 minus 5 cents (2 octaves below flame tone)
    delay(0.1 * 1000);
    noTone(sensorBuzzerPin);
  }
  else {
    Serial.println("Object Undetected");
  }
  delay(50);
 }

/*
 * Displays if a LoRa packet is received, and calls 3 functions:
 * the tempDetection(), flameDetection(), and irDetection().
 */
void loop() {

  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.println("Received packet ");

    //read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    String arr[2];
    int pos = 0;
    for (int i = 0; i < LoRaData.length(); i++) {
      if (LoRaData[i] == ';') {
        pos++;
      } else {
        arr[pos] = arr[pos] + LoRaData[i];
      }
    }

    temp_limit = arr[0].toInt();
    flame_limit = arr[1].toInt();

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rssi);

   // Dsiplay information
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(LoRaData);
   display.setCursor(0,40);
   display.print("RSSI:");
   display.setCursor(30,40);
   display.print(rssi);
   display.display();   
  } else {
   Serial.println("no packet.");
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("no packet.");
  }

  delay(200);

  // The three sensor functions
  tempDetection();
  flameDetection();
  irDetection();
  
}
