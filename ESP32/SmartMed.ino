#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_PWMServoDriver.h>

// ---------------- WIFI ----------------
#define WIFI_SSID     "Roshan"
#define WIFI_PASSWORD "12345678"
#define SERVER_URL    "https://smart-medicine-reminder-backend.onrender.com"  
#define DEVICE_ID     "DEVICE_001"

// ---------------- OBJECTS ----------------
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ---------------- PINS ----------------
#define BUZZER 25
#define IR_MORNING 32
#define IR_AFTERNOON 33
#define IR_NIGHT 34

#define SERVO_MIN 150
#define SERVO_MAX 600

// ---------------- SLOT STRUCT ----------------
struct DoseSlot {
  int hour;
  int minute;
  bool enabled;
  bool done;
};

DoseSlot slots[6];
// 0 M-before | 1 M-after | 2 A-before | 3 A-after | 4 N-before | 5 N-after

// ---------------- TIMERS ----------------
unsigned long lastDoseFetch = 0;
unsigned long messageStartTime = 0;
bool showingMessage = false;

// ---------------- DISPLAY HELPERS ----------------
String getMealDisplayName(int slotIndex) {
  switch (slotIndex / 2) {
    case 0: return "Breakfast";
    case 1: return "Lunch";
    case 2: return "Dinner";
    default: return "Meal";
  }
}

String getTimingDisplay(int slotIndex) {
  return (slotIndex % 2 == 0) ? "Before" : "After";
}

// ---------------- WIFI ----------------
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.clear();
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1500);
  lcd.clear();
}

// ---------------- BASIC FUNCTIONS ----------------
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(1500);
  digitalWrite(BUZZER, LOW);
}

bool waitForHand(int pin) {
  unsigned long start = millis();
  while (millis() - start < 60000) {
    if (digitalRead(pin) == LOW) return true;
  }
  return false;
}

void openBox(int channel) {
  pwm.setPWM(channel, 0, SERVO_MAX);
  delay(3000);
  pwm.setPWM(channel, 0, SERVO_MIN);
}

// ---------------- LCD DISPLAY (YOUR PREFERRED VERSION) ----------------
void showDoseInfo(String meal, String timing) {
  lcd.clear();             
  lcd.setCursor(0,0);
  lcd.print("Take Medicine");
  lcd.setCursor(0,1);

  if (meal == "morning" && timing == "before")
    lcd.print("Before BreakFast");
  else if (meal == "morning" && timing == "after")
    lcd.print("After BreakFast!");
  else if (meal == "afternoon" && timing == "before")
    lcd.print("Before Lunch!");
  else if (meal == "afternoon" && timing == "after")
    lcd.print("After Lunch!");
  else if (meal == "night" && timing == "before")
    lcd.print("Before Dinner!");
  else if (meal == "night" && timing == "after")
    lcd.print("After Dinner!");
}

int findNextDose(DateTime now) {
  int current = now.hour() * 60 + now.minute();
  int best = 1440;
  int idx = -1;

  for (int i = 0; i < 6; i++) {
    if (!slots[i].enabled || slots[i].done) continue;
    int t = slots[i].hour * 60 + slots[i].minute;
    if (t > current && (t - current) < best) {
      best = t - current;
      idx = i;
    }
  }
  return idx;
}

void showNextDose(DateTime now) {
  lcd.clear();
  int i = findNextDose(now);

  if (i == -1) {
    lcd.print("No More Doses");
    lcd.setCursor(0, 1);
    lcd.print("For Today");
  } else {
    lcd.setCursor(0,0);                    
    lcd.print("Next Dose ");
    lcd.print(getTimingDisplay(i));       
    
    lcd.setCursor(0,1);                    
    lcd.print(getMealDisplayName(i));     
    lcd.print(" ");
    lcd.printf("%02d:%02d", slots[i].hour, slots[i].minute);
  }
}

// ---------------- BACKEND LOG ----------------
void logDose(String meal, String timing, String schedTime, String status) {
  HTTPClient http;
  http.begin(String(SERVER_URL) + "/api/dose-log");
  http.addHeader("Content-Type", "application/json");

  DateTime now = rtc.now();

  StaticJsonDocument<256> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["date"] = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
  doc["meal"] = meal;
  doc["timing"] = timing;
  doc["scheduledTime"] = schedTime;
  doc["status"] = status;

  String payload;
  serializeJson(doc, payload);
  http.POST(payload);
  http.end();
}

// ---------------- FETCH DOSE TIMES ----------------
void fetchDoseTimes() {
  HTTPClient http;
  http.begin(String(SERVER_URL) + "/api/dose-time");

  if (http.GET() == 200) {
    StaticJsonDocument<512> doc;

    DeserializationError err = deserializeJson(doc, http.getString());
    if (err) {
      Serial.println("DoseTime JSON parse failed");
      http.end();
      return;
    }

    auto parse = [&](int i, JsonVariant v) {
      if (!v.isNull()) {
        const char* t = v.as<const char*>();
        slots[i].hour = (t[0]-'0')*10 + (t[1]-'0');
        slots[i].minute = (t[3]-'0')*10 + (t[4]-'0');
        slots[i].enabled = true;
        slots[i].done = false;
      } else {
        slots[i].enabled = false;
      }
    };

    parse(0, doc["morning"]["before"]);
    parse(1, doc["morning"]["after"]);
    parse(2, doc["afternoon"]["before"]);
    parse(3, doc["afternoon"]["after"]);
    parse(4, doc["night"]["before"]);
    parse(5, doc["night"]["after"]);
  }
  http.end();
}

// ---------------- SETUP ----------------
void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  rtc.begin();
  pwm.begin();
  pwm.setPWMFreq(50);

  pinMode(BUZZER, OUTPUT);
  pinMode(IR_MORNING, INPUT);
  pinMode(IR_AFTERNOON, INPUT);
  pinMode(IR_NIGHT, INPUT);

  lcd.clear();
  lcd.print("Smart Medicine");
  lcd.setCursor(0, 1);
  lcd.print("Reminder System");
  delay(2000);
  lcd.clear();

  connectWiFi();
  fetchDoseTimes();
  lastDoseFetch = millis();
}

// ---------------- LOOP ----------------
void loop() {
  DateTime now = rtc.now();

  if (millis() - lastDoseFetch > 300000) {
    fetchDoseTimes();
    lastDoseFetch = millis();
  }

  static bool wasShowingMessage = false;
  if (!showingMessage && wasShowingMessage) {
    lcd.clear();
  }
  wasShowingMessage = showingMessage;

  if (!showingMessage) {
    lcd.setCursor(0, 0);
    lcd.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    lcd.setCursor(0, 1);
    lcd.printf("%02d/%02d/%04d", now.day(), now.month(), now.year());
  }

  for (int i = 0; i < 6; i++) {
    if (!slots[i].enabled || slots[i].done) continue;

    if (now.hour() == slots[i].hour && now.minute() == slots[i].minute) {
      String meal = (i < 2) ? "morning" : (i < 4) ? "afternoon" : "night";
      String timing = (i % 2 == 0) ? "before" : "after";

      int irPin = (i < 2) ? IR_MORNING : (i < 4) ? IR_AFTERNOON : IR_NIGHT;
      int servo = (i < 2) ? 0 : (i < 4) ? 1 : 2;

      char schedTime[6];
      sprintf(schedTime, "%02d:%02d", slots[i].hour, slots[i].minute);

      showDoseInfo(meal, timing);  
      beep();

      bool taken = waitForHand(irPin);

      lcd.clear();
      if (taken) {
        openBox(servo);
        lcd.print("Dose Taken");
        logDose(meal, timing, String(schedTime), "taken");
      } else {
        lcd.print("Dose Missed");
        logDose(meal, timing, String(schedTime), "missed");
      }

      messageStartTime = millis();
      showingMessage = true;
      slots[i].done = true;
    }
  }

  if (showingMessage && millis() - messageStartTime > 5000) {
    showNextDose(now);
    delay(3000);
    showingMessage = false;
  }

  if (now.hour() == 0 && now.minute() == 0 && now.second() < 2) {
    for (int i = 0; i < 6; i++) slots[i].done = false;
  }

  delay(500);
}
