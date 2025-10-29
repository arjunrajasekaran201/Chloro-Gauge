#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Define pins
#define RED_LED_PIN 15
#define BLUE_LED_PIN 13
#define NIR_LED_PIN 2
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL   4
#define BATTERY_PIN 34 
#define SCL 22
#define SDA 21
#define SENSOR_INT 17
#define SW_1 27
#define SW_2 25
#define SW_3 38
#define SW_4 36

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int chlorophyllValue = 45;
int waterValue = 25;
int batteryValue = 0;

// ---------- Battery Icon ----------
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

// ---------- Bold Text ----------
void drawBoldText(int x, int y, const char* text, uint16_t color, int textSize) {
  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
  tft.setCursor(x + 1, y);
  tft.print(text);
}

// ---------- Read Battery Level ----------
int readBatteryLevel() {
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / 4095.0) * 3.3 * 2.0;  // voltage divider
  int percent = map(voltage * 100, 300, 420, 0, 100); // 3.0V→0%, 4.2V→100%
  percent = constrain(percent, 0, 100);
  return percent;
}

// ---------- Draw Main Display ----------
void drawDisplay() {
  tft.fillScreen(ST77XX_BLACK);

  drawBatteryIcon(180, 5, batteryValue);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(130, 7);
  tft.printf("%d%%", batteryValue);

  drawBoldText(10, 60, "CHLOROPHYLL", ST77XX_GREEN, 2);
  tft.setCursor(180, 60);
  tft.setTextSize(2);
  tft.printf("- %d%%", chlorophyllValue);

  drawBoldText(10, 90, "WATER", ST77XX_CYAN, 2);
  tft.setCursor(120, 90);
  tft.setTextSize(2);
  tft.printf("     - %d%%", waterValue);
}

// ---------- Startup Splash Screen ----------
void showStartupScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 60);
  tft.print("CHLORO GAUGE");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  // LED Setup (all OFF)
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(NIR_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(NIR_LED_PIN, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW); // keep backlight OFF at startup

  tft.init(135, 240);
  tft.setRotation(1);

  // Show startup screen before enabling backlight
  showStartupScreen();
  digitalWrite(TFT_BL, HIGH); // Turn screen on only after splash is drawn (prevents flash)
  delay(5000);

  analogReadResolution(12);
}

// ---------- Loop ----------
void loop() {
  batteryValue = readBatteryLevel();
  drawDisplay();
  delay(20000); // refresh every 20 sec
}
