#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <esp32/rom/rtc.h>  // for deep sleep functions

// ---------- Pin definitions ----------
#define RED_LED_PIN 15
#define BLUE_LED_PIN 13
#define NIR_LED_PIN 2

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL   4

#define SW_1 27  // Soft power button

// ---------- TFT object ----------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ---------- Variables ----------
int chlorophyllValue = 45;
int waterValue = 25;

// ---------- Button timing ----------
unsigned long buttonPressTime = 0;
const unsigned long HOLD_TIME = 5000; // 5s hold for shutdown
bool buttonHeld = false;

// ---------- Bold text ----------
void drawBoldText(int x, int y, const char* text, uint16_t color, int textSize) {
  tft.setTextSize(textSize);
  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
  tft.setCursor(x + 1, y);
  tft.print(text);
}

// ---------- Draw main screen ----------
void drawMainScreen() {
  tft.fillScreen(ST77XX_BLACK);
  drawBoldText(10, 60, "CHLOROPHYLL", ST77XX_GREEN, 2);
  tft.setCursor(180, 60);
  tft.setTextSize(2);
  tft.printf("- %d%%", chlorophyllValue);

  drawBoldText(10, 90, "WATER", ST77XX_CYAN, 2);
  tft.setCursor(120, 90);
  tft.setTextSize(2);
  tft.printf("     - %d%%", waterValue);

  // Draw battery outline as static
  tft.drawRect(180, 5, 40, 20, ST77XX_WHITE);
  tft.fillRect(180 + 40, 5 + 5, 3, 10, ST77XX_WHITE);
}

// ---------- Splash screen ----------
void showStartupScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 60);
  tft.print("CHLORO GAUGE");
}

// ---------- Shutdown screen ----------
void showShutdownScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  tft.setCursor(60, 50);
  tft.print("BYE");

  // Countdown
  for (int i = HOLD_TIME / 1000; i >= 0; i--) {
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(1, 100);
    tft.printf("Shutting Down in: %d s", i);
    delay(1000);
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  // LEDs off
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(NIR_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(NIR_LED_PIN, LOW);

  // TFT setup
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  tft.init(135, 240);
  tft.setRotation(1);

  // Button setup
  pinMode(SW_1, INPUT_PULLUP);

  // Show splash screen
  showStartupScreen();
  digitalWrite(TFT_BL, HIGH);
  delay(5000);

  // Draw main screen static elements
  drawMainScreen();

  // Configure wakeup for soft power
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0); // wake on button press
}

// ---------- Loop ----------
void loop() {
  int btnState = digitalRead(SW_1);

  if (btnState == LOW && !buttonHeld) {
    buttonPressTime = millis();
    buttonHeld = true;
  }

  if (btnState == LOW && buttonHeld) {
    unsigned long heldTime = millis() - buttonPressTime;
    if (heldTime >= HOLD_TIME) {
      showShutdownScreen();
      delay(100);
    }
  }

  if (btnState == HIGH && buttonHeld) {
    unsigned long heldTime = millis() - buttonPressTime;
    buttonHeld = false;
    if (heldTime >= HOLD_TIME) {
      Serial.println("Shutting down...");

      // Turn off all LEDs before sleep
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      digitalWrite(NIR_LED_PIN, LOW);
      digitalWrite(TFT_BL, LOW);

      delay(50);
      esp_deep_sleep_start();
    }
  }

  delay(100);
}
