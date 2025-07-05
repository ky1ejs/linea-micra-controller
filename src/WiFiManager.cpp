#include "WiFiManager.h"

WiFiManager::WiFiManager(const String& ssid, const String& password, unsigned long timeout)
    : ssid(ssid),
      password(password),
      connected(false),
      last_connection_attempt(0),
      connection_timeout(timeout),
      current_state(WiFiConnectionState::Disconnected) {}

bool WiFiManager::connect() {
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    current_state = WiFiConnectionState::Connected;
    return true;
  }

  current_state = WiFiConnectionState::Connecting;
  Serial.println("Connecting to WiFi: " + ssid);
  WiFi.begin(ssid.c_str(), password.c_str());

  last_connection_attempt = millis();

  while (WiFi.status() != WL_CONNECTED && (millis() - last_connection_attempt) < connection_timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    current_state = WiFiConnectionState::Connected;
    Serial.println();
    Serial.println("WiFi connected successfully");
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("Signal strength: " + String(WiFi.RSSI()) + " dBm");

    handleConnectionEvent();
    return true;
  } else {
    connected = false;
    current_state = WiFiConnectionState::Disconnected;
    Serial.println();
    Serial.println("WiFi connection failed");
    return false;
  }
}

void WiFiManager::disconnect() {
  WiFi.disconnect();
  connected = false;
  current_state = WiFiConnectionState::Disconnected;
  Serial.println("WiFi disconnected");
}

void WiFiManager::loop() {
  if (connected && WiFi.status() != WL_CONNECTED) {
    connected = false;
    current_state = WiFiConnectionState::Disconnected;
    Serial.println("WiFi connection lost");
    handleConnectionEvent();
  } else if (!connected && WiFi.status() == WL_CONNECTED) {
    connected = true;
    current_state = WiFiConnectionState::Connected;
    Serial.println("WiFi reconnected");
    handleConnectionEvent();
  }
}

bool WiFiManager::isConnected() const {
  return connected && (WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getLocalIP() const {
  return WiFi.localIP().toString();
}

int WiFiManager::getSignalStrength() const {
  return WiFi.RSSI();
}

void WiFiManager::setConnectionCallback(WiFiConnectionCallback callback) {
  connection_callback = callback;
}

void WiFiManager::reconnect() {
  if (!isConnected()) {
    current_state = WiFiConnectionState::Reconnecting;
    Serial.println("Attempting to reconnect to WiFi...");
    connect();
  }
}

void WiFiManager::handleConnectionEvent() {
  if (connection_callback) {
    connection_callback(connected);
  }
}

WiFiConnectionState WiFiManager::getConnectionState() const {
  return current_state;
}

String WiFiManager::getConnectionStatusText() const {
  switch (current_state) {
    case WiFiConnectionState::Disconnected:
      return "Disconnected";
    case WiFiConnectionState::Connecting:
      return "Connecting...";
    case WiFiConnectionState::Connected:
      return "Connected";
    case WiFiConnectionState::Reconnecting:
      return "Reconnecting...";
    default:
      return "Unknown";
  }
}