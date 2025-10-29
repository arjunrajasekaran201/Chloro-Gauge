#include <Wire.h>
#include "Adafruit_TCS34725.h"

// TCS34725 color sensor object
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_50MS,
    TCS34725_GAIN_4X
);

bool logging = false;          // flag to start/stop logging
unsigned long startTime = 0;   // time when logging starts

void setup() {
  Serial.begin(115200);

  if (!tcs.begin()) {
    Serial.println("No TCS34725 found");
    while (1); 
  }

  Serial.println("Press any key to start 5s logging...");
}

void loop() {
  // Wait for key press to start logging
  if (!logging && Serial.available() > 0) {
    Serial.read();  // clear the key press
    Serial.println("Time(ms),R,G,B");  // CSV header
    logging = true;
    startTime = millis();
  }

  if (logging) {
    if (millis() - startTime <= 5000) {  // log for 5 seconds
      float red, green, blue;

      tcs.setInterrupt(false);   // turn on sensor LED
      delay(60);                 // wait for reading
      tcs.getRGB(&red, &green, &blue);
      tcs.setInterrupt(true);    // turn off sensor LED

      // Print CSV format
      Serial.print(millis() - startTime);
      Serial.print(",");
      Serial.print((int)red);
      Serial.print(",");
      Serial.print((int)green);
      Serial.print(",");
      Serial.println((int)blue);

      delay(100);  // sample rate ~10Hz
    } else {
      Serial.println("Logging finished.");
      logging = false;  // stop logging until another key press
      Serial.println("Press any key to restart logging...");
    }
  }
}
