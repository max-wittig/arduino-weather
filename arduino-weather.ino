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
  const char* forecast;
} Location;
int numberOfLocations = 0;

Location locations[5];
int currentIndex = 0;

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
  lcd.print(loc->forecast);
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
    JsonArray root = doc.as<JsonArray>();
    for (JsonObject object : root) {
      locations[numberOfLocations] = Location{object["city"], object["temp"], object["forecast"]};
      numberOfLocations++;
    }
  }
  hasNewWeatherData = true;
}
