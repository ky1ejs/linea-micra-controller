#include <Adafruit_seesaw.h>
#include <Arduino.h>

#include "HomeAssistantClient.h"
#include "LaMarzoccoCloudClient.h"
#include "LineaMicra.h"
#include "RotaryEncoder.h"
#include "WiFiManager.h"
#include "config.h"
#include "display/DisplayManager.h"

int32_t encoder_position;

HomeAssistantClient haClient(HA_HOST, HA_PORT, HA_TOKEN);
LaMarzoccoCloudClient cloudClient(LM_USERNAME, LM_PASSWORD, LM_SERIAL_NUMBER);
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT);
LineaMicra* lineaMicra = nullptr;
DisplayManager display(240, 240);

RotaryEncoder<0> encoder1(ENCODER_A_PIN, ENCODER_B_PIN);

#define DISPLAY_ADDR 0x3C

void drawWiFiStrength(DisplayManager& display, int x, int y, int strength) {
  // Draw 4 bars of increasing height
  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2;
    if (strength > i) {
      display.fillRect(x + i * 3, y - barHeight, 2, barHeight, DisplayColor::WHITE);
    } else {
      display.drawRect(x + i * 3, y - barHeight, 2, barHeight, DisplayColor::WHITE);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("\nBooting...");

  encoder1.begin([](int id, int pin_a, int pin_b) {
    Serial.printf("Encoder %d initialized on pins A: %d, B: %d\n", id, pin_a, pin_b);
  });
  Serial.println("Encoder 1 initialized");

  // Initialize the display
  if (!display.init()) {
    Serial.println("Display initialization failed - continuing without display");
  } else {
    Serial.println("Display initialized successfully");
  }

  // Initialize WiFi
  display.setRotation(1);
  display.clear();
  display.setTextSize(2);
  display.setTextColor(DisplayColor::WHITE);
  display.setCursor(0, 0);
  display.setTextWrap(true);
  display.println("Connecting to WiFi...");
  display.println(WIFI_SSID);
  display.display();
  if (!wifiManager.connect()) {
    Serial.println("Failed to connect to WiFi");
    while (1)
      delay(10);
  }
  Serial.printf("WiFi connected with IP: %s", WiFi.localIP().toString().c_str());
  display.clear();
  display.println("Connected to WiFi");
  display.println(WiFi.localIP().toString());

  // Set up WiFi connection callback to handle HA reconnection
  wifiManager.setConnectionCallback([](bool connected) {
    if (connected) {
      Serial.println("WiFi reconnected, attempting to reconnect Home Assistant...");
      haClient.reconnect();
    } else {
      Serial.println("WiFi disconnected, Home Assistant will attempt reconnection when WiFi returns");
    }
  });

  // Initialize both Home Assistant and Cloud API clients
  display.println("Starting HA client...");
  display.display();
  delay(1000);

  if (!haClient.connect()) {
    Serial.println("Failed to connect to Home Assistant");
    while (1)
      delay(10);
  }
  Serial.println("Home Assistant client initialized");

  display.clear();
  display.setCursor(0, 0);
  display.println("Starting Cloud client...");
  display.display();
  delay(1000);

  if (!cloudClient.authenticate()) {
    Serial.println("Failed to authenticate with LaMarzocco Cloud API");
    while (1)
      delay(10);
  }
  Serial.println("LaMarzocco Cloud client initialized");

  display.clear();
  display.println("Connected to both");
  display.println("HA + Cloud API");
  display.display();
  delay(1000);
}

void loop() {
  display.clear();

  wifiManager.loop();  // Handle WiFi events
  haClient.loop();     // Handle Home Assistant events

  // Initialize LineaMicra if not already done (both clients must be ready)
  if (haClient.isConnected() && cloudClient.isAuthenticated()) {
    if (lineaMicra == nullptr) {
      lineaMicra = new LineaMicra(&haClient, &cloudClient);
      lineaMicra->fetchInitialState();  // Fetch initial state from cloud
      Serial.println("Linea Micra initialized with dual-client mode");
    }

    // Call LineaMicra's loop to handle debouncing
    lineaMicra->loop();

    // Handle rotary encoder temperature control
    static long lastEncoderValue = 0;
    long currentEncoderValue = encoder1.getValue();
    if (currentEncoderValue != lastEncoderValue) {
      long encoderDelta = currentEncoderValue - lastEncoderValue;
      float currentTemp = lineaMicra->getBoilerTemperature();
      float newTemp = currentTemp + (encoderDelta * 0.1f);
      
      // Constrain temperature to reasonable espresso range (80-100°C)
      newTemp = constrain(newTemp, 80.0f, 100.0f);
      
      if (newTemp != currentTemp) {
        lineaMicra->setBoilerTemperature(newTemp);
        Serial.printf("Encoder turned: %ld -> %ld (delta: %ld), Temperature: %.1f -> %.1f°C\n", lastEncoderValue,
                      currentEncoderValue, encoderDelta, currentTemp, newTemp);
      }
      
      lastEncoderValue = currentEncoderValue;
    }

    bool micraIsOn = lineaMicra->isOn();
    float boilerTemp = lineaMicra->getBoilerTemperature();
    bool preBrewOn = lineaMicra->isPreBrewOn();
    float preBrewTime = lineaMicra->getPreBrewTime();
    float preBrewWait = lineaMicra->getPreBrewWait();

    // Display values with pending state indication display.print("Linea Micra is ");
    if (lineaMicra->isPowerPending()) {
      display.print("*");  // Asterisk indicates pending
    }
    display.println(micraIsOn ? "ON" : "OFF");

    display.print("Temp: ");
    if (lineaMicra->isBoilerTemperaturePending()) {
      display.print("*");  // Asterisk indicates pending
    }
    display.println(String(boilerTemp) + " C");

    display.print("Prebrew: ");
    if (lineaMicra->isPreBrewModePending()) {
      display.print("*");  // Asterisk indicates pending
    }
    display.println(preBrewOn ? "ON" : "OFF");

    display.print("PB Time: ");
    if (lineaMicra->isPreBrewTimesPending()) {
      display.print("*");  // Asterisk indicates pending
    }
    display.println(String(preBrewTime) + " s");

    display.print("PB Wait: ");
    if (lineaMicra->isPreBrewTimesPending()) {
      display.print("*");  // Asterisk indicates pending
    }
    display.println(String(preBrewWait) + " s");

    display.println("Mode: HA + Cloud");
  } else {
    display.println("Linea Micra not ready");

    // Show WiFi status
    if (!wifiManager.isConnected()) {
      display.println("WiFi: " + wifiManager.getConnectionStatusText());
    }

    // Show HA status
    if (!haClient.isConnected()) {
      if (wifiManager.isConnected()) {
        display.println("HA: Reconnecting...");
      } else {
        display.println("HA: Waiting for WiFi");
      }
    }

    // Show Cloud status
    if (!cloudClient.isAuthenticated()) {
      display.println("Cloud not authenticated");
    }
  }

  // Store current counts (disable interrupts briefly for atomic read)
  int32_t enc1 = encoder1.getValue();
  Serial.printf("Encoder 1 position: %ld\n", enc1);

  // Draw WiFi signal strength
  int wifiStrength = WiFi.RSSI() / -20;  // Convert RSSI to WiFi strength (0-4)

  drawWiFiStrength(display, 0, 64, wifiStrength);
  display.display();  // update the display

  delay(100);  // wait 100 milliseconds for next scan
}
