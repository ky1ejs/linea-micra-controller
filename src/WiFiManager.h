#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

typedef std::function<void(bool)> WiFiConnectionCallback;

class WiFiManager {
private:
  String ssid;
  String password;
  bool connected;
  unsigned long last_connection_attempt;
  unsigned long connection_timeout;
  WiFiConnectionCallback connection_callback;

  void handleConnectionEvent();

public:
  WiFiManager(const String& ssid, const String& password, unsigned long timeout = 10000);

  bool connect();
  void disconnect();
  void loop();
  bool isConnected() const;
  String getLocalIP() const;
  int getSignalStrength() const;

  void setConnectionCallback(WiFiConnectionCallback callback);
  void reconnect();
};

#endif