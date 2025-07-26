#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi Credentials
const char* ssid = "IUB Guest";
const char* password = "IUBGuest";

// Firebase Config
const String FIREBASE_HOST = "https://<your-project>.firebaseio.com/";
const String FIREBASE_AUTH = "<your-database-secret>";

#define RXD2 16
#define TXD2 17

// Rotary Encoder Pins
#define ENCODER_DT   4
#define ENCODER_CLK  5
#define ENCODER_SW   15

// IR Sensor Pin
#define IR_SENSOR_PIN 26
bool irTriggered = false;

// Buzzer Pin
#define BUZZER_PIN 13

volatile int encoderValue = 1;
int lastEncoded = 0;
bool pressed = false;
String latestTag = "";
bool tagScanned = false;
unsigned long lastDebounce = 0;
unsigned long lastEncoderActivityTime = 0;
unsigned long noScanStartTime = 0;

// Emergency Button Press Tracking
unsigned long buttonPressStart = 0;
bool buttonHeld = false;

void IRAM_ATTR handleEncoder() {
  int dtValue = digitalRead(ENCODER_DT);
  int clkValue = digitalRead(ENCODER_CLK);
  if (dtValue != clkValue) encoderValue++;
  else encoderValue--;
  if (encoderValue < 1) encoderValue = 1;
  lastEncoderActivityTime = millis();
}

void IRAM_ATTR handleButton() {
  if (millis() - lastDebounce > 200) {
    buttonPressStart = millis();
    pressed = true;
    lastDebounce = millis();
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  display.clearDisplay();
  display.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    display.println("WiFi Connected");
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connection Failed!");
    display.println("WiFi Connect Fail");
  }
  display.display();
}

void waitForIR() {
  irTriggered = false;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Waiting for IR...");
  display.display();
  while (!irTriggered) {
    if (digitalRead(IR_SENSOR_PIN) == LOW) {
      irTriggered = true;
      Serial.println("IR Sensor Triggered! Starting program...");
      break;
    }
    delay(100);
  }
}

void initializeSystem() {
  connectToWiFi();
  delay(1000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("       Welcome");
  display.println("");
  display.println("1. Ali Muneer");
  display.println("2. Mohib Hassan");
  display.println("3. Muhammad Ikram");
  display.println("4. Supervised by:");
  display.println("   Dr. Abdur Raheem");
  display.display();
  delay(3000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("RFID Ready...");
  display.setCursor(0, 20);
  display.print("Qty: ");
  display.println(encoderValue);
  display.display();

  Serial.println("RFID Reader ready...");
  lastEncoderActivityTime = millis();
  noScanStartTime = millis();
  tagScanned = false;
  pressed = false;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  display.begin(SCREEN_ADDRESS, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  waitForIR();
  initializeSystem();

  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handleButton, FALLING);
}

void loop() {
  // Increased to 1 minute timeout (60000 ms)
  if (millis() - noScanStartTime > 60000 && !tagScanned) {
    Serial.println("No scan for 1 minute. Going to idle...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("System Idle...");
    display.display();
    waitForIR();        // Wait for IR sensor again
    initializeSystem(); // Reinitialize
  }

  if (Serial2.available()) {
    String rfid = Serial2.readStringUntil('\n');
    rfid.trim();
    latestTag = rfid;
    encoderValue = 1;
    tagScanned = true;
    pressed = false;
    lastEncoderActivityTime = millis();
    noScanStartTime = millis();  // Reset timer

    Serial.print("RFID Tag: ");
    Serial.println(rfid);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("RFID Ready...");
    display.setCursor(0, 20);
    display.print("Tag: ");
    display.println(rfid);
    display.setCursor(0, 40);
    display.print("Qty: ");
    display.println(encoderValue);
    display.display();
  }

  if (digitalRead(ENCODER_SW) == LOW) {
    if (!buttonHeld && millis() - buttonPressStart > 1500) {
      buttonHeld = true;
      Serial.println("EMERGENCY BUTTON HELD!");

      // Show Emergency alert and beep for 3 seconds, no Firebase send
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("!!! EMERGENCY !!!");
      display.display();

      tone(BUZZER_PIN, 1000);  // Start beep tone
      delay(3000);
      noTone(BUZZER_PIN);      // Stop beep tone

      // Reset state like before
      encoderValue = 1;
      tagScanned = false;
      pressed = false;

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("RFID Ready...");
      display.setCursor(0, 20);
      display.print("Qty: ");
      display.println(encoderValue);
      display.display();

      noScanStartTime = millis(); // Reset inactivity timer
    }
  } else {
    buttonHeld = false;
  }

  if (tagScanned && !pressed) {
    display.setCursor(0, 40);
    display.fillRect(0, 40, 128, 10, SH110X_BLACK);
    display.setCursor(0, 40);
    display.print("Qty: ");
    display.println(encoderValue);
    display.display();
    delay(100);
  }

  if (tagScanned && !pressed && (millis() - lastEncoderActivityTime > 4000)) {
    String payload = "{\"tag\":\"" + latestTag + "\",\"qty\":" + String(encoderValue) + "}";
    sendToFirebase(payload);
    encoderValue = 1;
    tagScanned = false;
    pressed = false;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("RFID Ready...");
    display.setCursor(0, 20);
    display.print("Qty: ");
    display.println(encoderValue);
    display.display();

    noScanStartTime = millis();  // Reset inactivity timer
  }

  if (tagScanned && pressed) {
    String payload = "{\"tag\":\"" + latestTag + "\",\"qty\":" + String(encoderValue) + "}";
    sendToFirebase(payload);
    encoderValue = 1;
    tagScanned = false;
    pressed = false;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("RFID Ready...");
    display.setCursor(0, 20);
    display.print("Qty: ");
    display.println(encoderValue);
    display.display();

    noScanStartTime = millis();  // Reset inactivity timer
  }
}

void sendToFirebase(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = FIREBASE_HOST + "rfidLogs.json?auth=" + FIREBASE_AUTH;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Firebase Response: ");
      Serial.println(http.getString());
      tone(BUZZER_PIN, 1000, 200);  // Success tone
    } else {
      Serial.print("Error sending to Firebase: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}

void sendToFirebaseEmergency(String payload) {
  // Removed usage as per request: no sending emergency data now
  // Keeping the function for completeness, but it is never called now.
}
