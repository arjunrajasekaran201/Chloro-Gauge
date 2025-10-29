#define RED_LED   15
#define NIR_LED    2
#define BLUE_LED  13

void setup() {
  // Set LED pins as outputs
  pinMode(RED_LED, OUTPUT);
  pinMode(NIR_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}

void loop() {
  // Turn all ON
  digitalWrite(RED_LED, HIGH);
  digitalWrite(NIR_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  delay(500); // keep ON for 0.5 sec

  // Turn all OFF
  digitalWrite(RED_LED, LOW);
  digitalWrite(NIR_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  delay(500); // keep OFF for 0.5 sec
}
