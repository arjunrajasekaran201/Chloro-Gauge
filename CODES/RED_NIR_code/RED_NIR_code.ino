#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// Pin definitions for ESP32
#define RED_LED 15
#define IR_LED 2
#define SDA_PIN 21
#define SCL_PIN 22

// Create TSL2561 sensor object
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// Variables for readings
uint16_t ired, iir, iredMax, iirMax;
float transRed, transIR, cci;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initializing Chlorophyll Index Measurement...");

  // Initialize I2C on ESP32 pins
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(RED_LED, OUTPUT);
  pinMode(IR_LED, OUTPUT);

  // Initialize the TSL2561 sensor
  if (!tsl.begin()) {
    Serial.println("TSL2561 sensor not found! Check wiring and power.");
    while (1);
  }

  configureSensor();

  // ---- Calibration phase ----
  Serial.println("Calibrating RED LED...");
  iredMax = measureLED(RED_LED);
  Serial.print("Red Reference Value (Raw): ");
  Serial.println(iredMax);

  Serial.println("Calibrating IR LED...");
  iirMax = measureLED(IR_LED);
  Serial.print("IR Reference Value (Raw): ");
  Serial.println(iirMax);

  Serial.println("\nCalibration complete. Ready for leaf measurement.\n");
}

void loop() {
  Serial.println("Measuring leaf sample...");

  // Measure LEDs sequentially
  ired = measureLED(RED_LED);
  iir = measureLED(IR_LED);

  // Avoid division by zero
  if (iredMax == 0 || iirMax == 0) {
    Serial.println("Calibration error: reference values invalid!");
    delay(2000);
    return;
  }

  // Compute transmission ratios and chlorophyll index
  transRed = ((float)ired / iredMax) * 100.0;
  transIR = ((float)iir / iirMax) * 100.0;
  cci = transIR / transRed;

  // Print results
  Serial.print("Red: ");
  Serial.print(ired);
  Serial.print(" | IR: ");
  Serial.print(iir);
  Serial.print(" | Chlorophyll Index (CCI): ");
  Serial.println(cci, 2);

  Serial.println("-----------------------------------");
  delay(5000); // Wait before next reading
}

// Measure raw sensor value for given LED
uint16_t measureLED(uint8_t ledPin) {
  digitalWrite(RED_LED, LOW);
  digitalWrite(IR_LED, LOW);

  digitalWrite(ledPin, HIGH);
  delay(500); // Allow LED and sensor to stabilize

  uint16_t broadband, infrared;
  tsl.getLuminosity(&broadband, &infrared);

  digitalWrite(ledPin, LOW);
  return broadband; // Use broadband channel as main reading
}

// Configure TSL2561
void configureSensor() {
  tsl.enableAutoRange(true); // Auto-gain
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS); // Max sensitivity
  Serial.println("Sensor configured: Auto-range ON, Integration time = 402 ms");
}
