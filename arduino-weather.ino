#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

String currentText = "";
String line1Text = "";
String line2Text = "";
const int contrastPin = A5;
const int switchPin = 13;
const int backgroundPin = 8;

boolean backgroundEnabled = true;
boolean switchState = true;
boolean inputAccepted = true;

void setup() {
  pinMode(contrastPin, OUTPUT);
  pinMode(backgroundPin, OUTPUT);
  pinMode(switchPin, INPUT);
  analogWrite(contrastPin, 100);
  digitalWrite(backgroundPin, HIGH);
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop() {
  if (inputAccepted && digitalRead(switchPin)) {
    inputAccepted = false;
    backgroundEnabled = !backgroundEnabled;
    if (backgroundEnabled) {
      digitalWrite(backgroundPin, HIGH);
    } else {
      digitalWrite(backgroundPin, LOW);
    }
    delay(500);
    inputAccepted = true;
  }
}

void serialEvent() {
  lcd.setCursor(0, 0);
  currentText = Serial.readString();
  if (currentText != "" && currentText != NULL) {
    line1Text = currentText.substring(0, 16);
    line2Text = currentText.substring(16, 32);
    lcd.print(line1Text);

    if (16 < currentText.length()) {
      lcd.setCursor(0, 1);
      lcd.print(line2Text);
    }
  }
}
