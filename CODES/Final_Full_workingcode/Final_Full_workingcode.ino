#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <esp32/rom/rtc.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

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
#define SW_1 27
#define MEASURE_BTN 25
#define BATTERY_PIN 34
#define VOLTAGE_DIVIDER_SCALING 2.0

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

uint16_t iredMax, iblueMax, iirMax;
float cciValue = -1;

unsigned long buttonPressTime = 0;
const unsigned long HOLD_TIME = 5000;
bool buttonHeld = false;
bool lastMeasureBtnState = HIGH;

const unsigned char lightning_bolt_24x24[] PROGMEM = {
  0b00000000,0b01000000,0b00000000,
  0b00000000,0b01100000,0b00000000,
  0b00000000,0b11100000,0b00000000,
  0b00000001,0b11100000,0b00000000,
  0b00000001,0b11110000,0b00000000,
  0b00000001,0b11110000,0b00000000,
  0b00000000,0b11110000,0b00000000,
  0b00000000,0b11110000,0b00000000,
  0b00000000,0b11100000,0b00000000,
  0b00000000,0b11100000,0b00000000,
  0b00000000,0b01110000,0b00000000,
  0b00000000,0b00110000,0b00000000,
  0b00000000,0b00110000,0b00000000,
  0b00000000,0b00110000,0b00000000,
  0b00000000,0b01100000,0b00000000,
  0b00000000,0b01000000,0b00000000,
  0b00000000,0b01000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000,
  0b00000000,0b00000000,0b00000000
};

void drawLightningBitmap(int x, int y, uint16_t color) {
  tft.drawBitmap(x, y, lightning_bolt_24x24, 24, 24, color);
}

float readBatteryVoltage() {
  uint8_t samples = 30;
  int sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += analogRead(BATTERY_PIN);
    delay(3);
  }
  float avg = (float)sum / samples;
  float vout = (avg * 3.3) / 4096.0;
  return vout * VOLTAGE_DIVIDER_SCALING;
}

int readBatteryPercent() {
  float voltage = readBatteryVoltage();
  int percent = map(voltage * 100, 300, 420, 0, 100);
  return constrain(percent, 0, 100);
}

void drawBatteryIcon(int x, int y, int level) {
  tft.drawRect(x, y, 40, 20, ST77XX_WHITE);
  tft.fillRect(x + 40, y + 5, 3, 10, ST77XX_WHITE);
  uint16_t color;
  if (level > 50) color = ST77XX_GREEN;
  else if (level > 20) color = ST77XX_YELLOW;
  else color = ST77XX_RED;
  int fillWidth = map(level, 0, 100, 0, 38);
  tft.fillRect(x + 1, y + 1, fillWidth, 18, color);
}

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
  int batteryPercent = readBatteryPercent();
  float batteryVoltage = readBatteryVoltage();
  bool showLightning = false;
  if (batteryVoltage > 3.89) {
    batteryPercent -= 20;
    if (batteryPercent < 0) batteryPercent = 0;
    showLightning = true;
  }
  drawBatteryIcon(185, 5, batteryPercent);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(145, 8);
  tft.printf("%d%%", batteryPercent);
  if (showLightning) drawLightningBitmap(130, 7, ST77XX_YELLOW);

  drawBoldText(10, 60, "CCI INDEX", ST77XX_GREEN, 2);
  tft.setTextSize(2);
  tft.setCursor(140, 60);
  if (cciValue < 0) tft.print("-");
  else tft.printf(": %.2f", cciValue);
}

void updateCCIDisplay(float value) {
  cciValue = value;
  drawMainScreen();
}

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

void showStartupScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 60);
  tft.print("CHLORO GAUGE");
}

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
    tft.printf("TURNING OFF IN: %ds", i);
    delay(1000);
  }
}

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

unsigned long lastUpdate = 0;
const unsigned long REFRESH_INTERVAL = 20000;

void loop() {
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

  bool currentState = digitalRead(MEASURE_BTN);
  if (lastMeasureBtnState == HIGH && currentState == LOW) {
    uint16_t r = measureLED(SENSOR_RED_LED);
    uint16_t b = measureLED(SENSOR_BLUE_LED);
    uint16_t ir = measureLED(SENSOR_IR_LED);
    float transRed = (float)r / iredMax;
    float transBlue = (float)b / iblueMax;
    float transIR  = (float)ir / iirMax;
    float cci = transIR / (transRed + transBlue);
    updateCCIDisplay(cci);
  }
  lastMeasureBtnState = currentState;

  if (millis() - lastUpdate >= REFRESH_INTERVAL) {
    drawMainScreen();
    lastUpdate = millis();
  }
  delay(50);
}
