// SOEN 422 Project: Helmet add-on for emergency workers
/*
 * The NanoFan file is for the Arduino Nano connected to the HelmetUsers TTGO
 * through serial communication. Its purpose is to continuously run the
 * servo (fan) while the TTGO can check the thresholds without stopping. This
 * allows the servo (fan) to spin continuously. If our servo was connected directly
 * to the TTGO, it could not monitor the other thresholds while also spinning
 * the fan.
 * 
 * This code was developed by Simon Bilodeau, Amine Halimi, and Michael Rowe
 */

#include <Servo.h>
#include "ArduinoJson.h"
#include <SoftwareSerial.h>

int pos = 0;
Servo servo_12;
int value = 48;

SoftwareSerial mySerial(4,5); 
/*
 * sets up servo connections and serial connection to TTGO
 */
void setup() {
  Serial.begin(115200);
  servo_12.attach(12, 500, 2500);
  servo_12.write(90);

  mySerial.begin(9600);
}

/*
 * Checks the received value from the TTGO, spins if required
 */
void loop() {
  // Check if the other Arduino is transmitting
  if (mySerial.available()) {
    Serial.print("Receiving ");
    value = mySerial.read();
    Serial.println(value);
  }

  else {
      Serial.println("Nope");
  }
  /*
   * If the TTGO sends a 1 to the Nano, the Nano reads this as 49.
   * This activates the servo.
   */
  if (value == 49) {
    Serial.println("\nHotter than 28 C!\n");
    // sweep the servo from 0 to 180 degrees in steps
    // of 1 degrees
    for (pos = 0; pos <= 180; pos += 20) {
      // tell servo to go to position in variable 'pos'
      servo_12.write(pos);
      // wait 15 ms for servo to reach the position
      delay(100); // Wait for 15 millisecond(s)
    }
    for (pos = 180; pos >= 0; pos -= 20) {
      // tell servo to go to position in variable 'pos'
      servo_12.write(pos);
      // wait 15 ms for servo to reach the position
      delay(100); // Wait for 15 millisecond(s)
    }
  } else { // servo is not activated.
    servo_12.write(0);
    delay(1000);
  }
}
