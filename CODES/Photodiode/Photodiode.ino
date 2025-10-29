// ESP32 RED LED Pulse + Photodiode Reader
// LED pulses: 0.25s ON, 0.5s OFF
// Photodiode constantly sampled, outputs value if >2700 else 0

#define LED_PIN  12      // Red LED connected here
#define PHOTO_PIN  4    // Photodiode analog input (use ADC pin, e.g. GPIO34)

unsigned long lastToggle = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // LED pulsing logic
  if (!ledState && (currentMillis - lastToggle >= 1000)) {
    // Turn ON LED for 250ms
    digitalWrite(LED_PIN, HIGH);
    ledState = true;
    lastToggle = currentMillis;
  } 
  else if (ledState && (currentMillis - lastToggle >= 500)) {
    // Turn OFF LED for 500ms
    digitalWrite(LED_PIN, LOW);
    ledState = false;
    lastToggle = currentMillis;
  }

  // Photodiode constant reading
  int rawValue = analogRead(PHOTO_PIN);

  if (rawValue > 2000) {
    Serial.println(rawValue);  // Show pulse
  } else {
    Serial.println(0);         // No pulse detected
  }

  delay(100); // small delay to stabilize readings
}
