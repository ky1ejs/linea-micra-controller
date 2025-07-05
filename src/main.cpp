#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_seesaw.h>
#include <Arduino.h>

#include "HomeAssistantClient.h"
#include "LineaMicra.h"
#include "RotaryEncoder.h"
#include "WiFiManager.h"
#include "config.h"
#include "i2cScanner.h"

Adafruit_seesaw ss;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
int32_t encoder_position;

RotaryEncoder<0> encoder1(0, 20);
HomeAssistantClient haClient(HA_HOST, HA_PORT, HA_TOKEN);
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT);
LineaMicra* lineaMicra = nullptr;

#define SEESAW_ADDR 0x36
#define SS_SWITCH 24
#define DISPLAY_ADDR 0x3C

void drawWiFiStrength(Adafruit_SH1107& display, int x, int y, int strength) {
  // Draw 4 bars of increasing height
  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2;
    if (strength > i) {
      display.fillRect(x + i * 3, y - barHeight, 2, barHeight, SH110X_WHITE);
    } else {
      display.drawRect(x + i * 3, y - barHeight, 2, barHeight, SH110X_WHITE);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("\nI2C Scanner");

  initI2C();

  encoder1.begin();
  Serial.println("Encoder 1 initialized");

  if (!ss.begin(SEESAW_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while (1)
      delay(10);
  }
  Serial.println("seesaw started");

  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  if (version != 4991) {
    Serial.print("Wrong firmware loaded? ");
    Serial.println(version);
    while (1)
      delay(10);
  }
  Serial.println("Found Product 4991");

  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  encoder_position = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();

  // Initialize the display
  if (!display.begin(DISPLAY_ADDR)) {
    Serial.println("SH110X allocation failed - continuing without display");
  } else {
    Serial.println("Display initialized successfully");
  }

  // Initialize WiFi
  display.setRotation(1);
  display.clearDisplay();
  display.setTextSize(1.2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextWrap(true);
  display.println("Connecting to WiFi...");
  display.display();
  if (!wifiManager.connect()) {
    Serial.println("Failed to connect to WiFi");
    while (1)
      delay(10);
  }
  Serial.printf("WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
  display.clearDisplay();
  display.println("Connected to WiFi");
  display.println(WiFi.localIP().toString());
  display.println("Starting HA client...");
  display.display();
  delay(1000);  // Give time for display to update

  if (!haClient.connect()) {
    Serial.println("Failed to connect to Home Assistant");
    while (1)
      delay(10);
  }
  Serial.println("Home Assistant client initialized");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected to Home Assistant");
  display.display();
  delay(1000);  // Give time for display to update
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);

  wifiManager.loop();  // Handle WiFi events
  haClient.loop();     // Handle Home Assistant events

  if (!ss.digitalRead(SS_SWITCH)) {
    Serial.println("Button pressed!");
  }

  // Read the encoder position
  if (haClient.isConnected()) {
    if (lineaMicra == nullptr) {
      lineaMicra = new LineaMicra(&haClient);
      Serial.println("Linea Micra initialized");
    }

    bool micraIsOn = lineaMicra->isOn();
    display.println("Linea Micra is " + String(micraIsOn ? "ON" : "OFF"));
    display.println("Temp: " + String(lineaMicra->getBoilerTemperature()) + " C");
    display.println("Prebrew: " + String(lineaMicra->isPreBrewOn() ? "ON" : "OFF"));
    display.println("Prebrew Time: " + String(lineaMicra->getPreBrewTime()) + " s");
    display.println("Prebrew Wait: " + String(lineaMicra->getPreBrewWait()) + " s");
  }

  // Store current counts (disable interrupts briefly for atomic read)
  int32_t enc1 = encoder1.getValue();
  int32_t new_position = ss.getEncoderPosition();

  // Draw WiFi signal strength
  int wifiStrength = WiFi.RSSI() / -20;  // Convert RSSI to WiFi strength (0-4)

  drawWiFiStrength(display, 0, 64, wifiStrength);
  // display.setCursor(0, 56);
  // display.print("WiFi Strength: ");
  // display.println(wifiStrength);
  display.display();  // update the display

  // scanI2CDevices();
  delay(100);  // wait 100 milliseconds for next scan
}
