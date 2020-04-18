#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <timer.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
auto timer = timer_create_default();

const int contrastPin = A5;
const int switchPin = 13;
const int backgroundPin = 8;

boolean backgroundEnabled = true;
boolean switchState = true;
boolean inputAccepted = true;

unsigned long previousMillis = 0;

typedef struct { 
  const char* name;
  const char* temp;
} Location;

Location locations[10]{};

String currentDate = "";
String currentLocationString = "";
boolean hasNewWeatherData = false;

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

  if (hasNewWeatherData) {
    hasNewWeatherData = false;
    cycleLocations();
  }
  timer.tick();
}

void cycleLocations() {
  int timeout = 0;
  for(const Location &location : locations) {
    timeout += 3000;
    currentLocationString = String(location.name) + " " + String(location.temp) + " C";
    timer.every(timeout, showOnDisplay);
  }
}

void showOnDisplay(void *) {
  lcd.setCursor(0, 0);
  lcd.print(currentLocationString);
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
    hasNewWeatherData = true;
  }
}
