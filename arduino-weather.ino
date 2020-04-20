#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <timer.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
Timer<1> timer;

const int contrastPin = A5;
const int switchPin = 13;
const int backgroundPin = 8;

boolean backgroundEnabled = true;
boolean switchState = true;
boolean inputAccepted = true;

typedef struct {
  const char* city;
  const char* temp;
} Location;
int numberOfLocations = 0;

Location locations[10];
int currentIndex = 0;

const char* currentDate = "";
boolean hasNewWeatherData = false;
Timer<>::Task currentTask;

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
    timer.cancel(currentTask);
    currentTask = timer.every(3000, showOnDisplay);
  }
  timer.tick();
}

bool showOnDisplay(void *) {
  lcd.clear();
  lcd.setCursor(0, 0);
  Location* loc = &locations[currentIndex];
  char line1Buffer[16];
  strcpy(line1Buffer, loc->city);
  strcat(line1Buffer, " ");
  strcat(line1Buffer, loc->temp);
  strcat(line1Buffer, " C");
  lcd.print(line1Buffer);
  lcd.setCursor(0, 1);
  lcd.print(currentDate);
  if (currentIndex < numberOfLocations-1) {
    currentIndex++;
  } else {
    currentIndex = 0;
  }
  return true;
}

void serialEvent() {
  String data = Serial.readString();
  if (data != "" && data != NULL) {
    // clear locations
    memset(locations, 0, sizeof(locations));
    numberOfLocations = 0;
    DynamicJsonDocument doc(300);
    deserializeJson(doc, data);
    JsonObject root = doc.as<JsonObject>();
    int i = 0;
    for (JsonPair p : root) {
      const char* key = p.key().c_str();
      const char* value = p.value();
      if (strcmp("date", key) == 0) {
        currentDate = (char*)value;
      } else {
        locations[i] = Location{key, value};
        i++;
        numberOfLocations++;
      }
    }
    hasNewWeatherData = true;
  }
}
