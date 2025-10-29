#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <esp32/rom/rtc.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// ---------- PIN DEFINITIONS ----------
#define RED_LED_PIN 15
#define BLUE_LED_PIN 13
#define NIR_LED_PIN 2

#define SENSOR_RED_LED 15
#define SENSOR_BLUE_LED 13
#define SENSOR_IR_LED 2

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL   4

#define SW_1 27         // Soft power button
#define MEASURE_BTN 25  // Measure chlorophyll button

// ---------- TFT ----------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ---------- Sensor ----------
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
uint16_t iredMax, iblueMax, iirMax;
float leafPercent = -1;

// ---------- Shutdown Button Timing ----------
unsigned long buttonPressTime = 0;
const unsigned long HOLD_TIME = 5000;
bool buttonHeld = false;
bool lastMeasureBtnState = HIGH;

// ---------- UI FUNCTIONS ----------
void drawBoldText(int x, int y, const char* text, uint16_t color, int textSize) {
  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
  tft.setCursor(x + 1, y);
  tft.print(text);
}

void drawMainScreen() {
  tft.fillScreen(ST77XX_BLACK);
  drawBoldText(10, 60, "CHLOROPHYLL", ST77XX_GREEN, 2);
  tft.setTextSize(2);
  tft.setCursor(180, 60);
  tft.print("- %");
}

void updateChlorophyllDisplay(float value) {
  tft.setCursor(180, 60);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (value < 0) tft.print("-  %");
  else tft.printf("%.1f%% ", value);
}

// ---------- LED Measurement ----------
uint16_t measureLED(uint8_t ledPin) {
  digitalWrite(SENSOR_RED_LED, LOW);
  digitalWrite(SENSOR_BLUE_LED, LOW);
  digitalWrite(SENSOR_IR_LED, LOW);
  digitalWrite(ledPin, HIGH);
  delay(500);

  uint16_t broadband, infrared;
  tsl.getLuminosity(&broadband, &infrared);

  digitalWrite(ledPin, LOW);
  return broadband;
}

// ---------- Splash ----------
void showStartupScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 60);
  tft.print("CHLORO GAUGE");
}

// ---------- Shutdown ----------
void showShutdownScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  tft.setCursor(60, 50);
  tft.print("BYE");
  for (int i = HOLD_TIME / 1000; i >= 0; i--) {
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(1, 100);
    tft.printf("Shutting Down in: %d s", i);
    delay(1000);
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(NIR_LED_PIN, OUTPUT);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);

  tft.init(135, 240);
  tft.setRotation(1);

  pinMode(SW_1, INPUT_PULLUP);
  pinMode(MEASURE_BTN, INPUT_PULLUP);

  Wire.begin(21, 22);

  tsl.begin();
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);

  Serial.println("Calibrating...");
  iredMax = measureLED(SENSOR_RED_LED);
  iblueMax = measureLED(SENSOR_BLUE_LED);
  iirMax = measureLED(SENSOR_IR_LED);

  showStartupScreen();
  digitalWrite(TFT_BL, HIGH);
  delay(3000);

  drawMainScreen();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0);
}

// ---------- LOOP ----------
void loop() {

  // Soft power button logic
  int btnState = digitalRead(SW_1);
  if (btnState == LOW && !buttonHeld) {
    buttonPressTime = millis();
    buttonHeld = true;
  }
  if (btnState == LOW && buttonHeld && millis() - buttonPressTime >= HOLD_TIME) {
    showShutdownScreen();
    esp_deep_sleep_start();
  }
  if (btnState == HIGH) buttonHeld = false;

  // ---- Measure Button (Single Press) ----
  bool currentState = digitalRead(MEASURE_BTN);
  if (lastMeasureBtnState == HIGH && currentState == LOW) {
    uint16_t r = measureLED(SENSOR_RED_LED);
    uint16_t b = measureLED(SENSOR_BLUE_LED);
    uint16_t ir = measureLED(SENSOR_IR_LED);

    float transRed = (float)r / iredMax;
    float transBlue = (float)b / iblueMax;
    float transIR  = (float)ir / iirMax;

    float leafIndex = transIR / (transRed + transBlue);
    leafPercent = 100 - leafIndex;

    updateChlorophyllDisplay(leafPercent);
  }

  lastMeasureBtnState = currentState;
  delay(50);
}
