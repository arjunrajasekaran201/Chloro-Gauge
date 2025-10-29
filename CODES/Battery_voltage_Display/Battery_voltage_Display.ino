// ESP32 TTGO T-DISPLAY Voltmeter using Adafruit_ST7789
// Pins 13, 15, 2, 17 forced LOW

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// --- Display Pins ---
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL   4   // Backlight control pin

// Create display object (Hardware SPI)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Analog input pin
#define analogInPin 34

// Pins to keep LOW always
#define P1 13
#define P2 15
#define P3 2
#define P4 17

void setup() {
  Serial.begin(115200);
  
  pinMode(analogInPin, INPUT);

  // Set pins low
  pinMode(P1, OUTPUT);
  pinMode(P2, OUTPUT);
  pinMode(P3, OUTPUT);
  pinMode(P4, OUTPUT);

  digitalWrite(P1, LOW);
  digitalWrite(P2, LOW);
  digitalWrite(P3, LOW);
  digitalWrite(P4, LOW);

  // Init SPI Display
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);
  tft.init(135, 240);  // TTGO T-Display resolution, rotate if needed
  tft.setRotation(1);
  
  // Turn on backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.println("Voltage Reading:");
}

void displayReading(float va) {
  // Clear old reading area
  tft.fillRect(0, 50, 240, 80, ST77XX_BLACK);

  tft.setCursor(50, 60);
  tft.setTextSize(3);
  tft.print(va, 2);
  tft.print(" V");
}

void loop() {
  uint8_t samples = 30;
  int sum = 0;

  while (samples--) {
    sum += analogRead(analogInPin);
    delay(3);
  }

  float avg = (float)sum / 30.0;
  float vout = (avg * 3.3) / 4096.0;
  float voltage = vout * 2.0; // Divider scaling

  displayReading(voltage);

  Serial.print("Voltage: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  // Keep pins LOW always
  digitalWrite(P1, LOW);
  digitalWrite(P2, LOW);
  digitalWrite(P3, LOW);
  digitalWrite(P4, LOW);

  delay(100);
}
