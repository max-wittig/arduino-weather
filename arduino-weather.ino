#include <LiquidCrystal.h>
#include <ArduinoJson.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const int contrastPin = A5;
const int switchPin = 13;
const int backgroundPin = 8;

boolean backgroundEnabled = false;
boolean switchState = true;
boolean inputAccepted = true;

String currentDate = "";

typedef struct { 
  const char* name;
  const char* temp;
} Location;

Location locations[10]{};

void setup() {
  pinMode(contrastPin, OUTPUT);
  pinMode(backgroundPin, OUTPUT);
  pinMode(switchPin, INPUT);
  analogWrite(contrastPin, 100);
  digitalWrite(backgroundPin, LOW);
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
  cycleLocations();
}

void cycleLocations() {
  for(const Location &location : locations) {
    showOnDisplay(String(location.name) + " " + String(location.temp) + " C");
    delay(5000);
  }
}

void showOnDisplay(String line1) {
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(currentDate);
}

void serialEvent() {
  String currentText = Serial.readString();
  if (currentText != "" && currentText != NULL) {
    // clear locations
    memset(locations, 0, sizeof(locations));
    DynamicJsonDocument doc(200);
    deserializeJson(doc, currentText);
    JsonObject root = doc.as<JsonObject>();
    int i = 0;
    for (JsonPair p : root) {
      const char* key = p.key().c_str();
      const char* value = p.value();
      if (key == "date") {
        currentDate = String(value);
      }

      locations[i] = Location{key, value};
      i++;
    }
  }
}
