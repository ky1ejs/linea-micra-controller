#ifndef HOME_ASSISTANT_CLIENT_H
#define HOME_ASSISTANT_CLIENT_H

#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>

#include <map>

using namespace websockets;

struct HAEntity {
  String entity_id;
  String state;
  String friendly_name;
  unsigned long last_updated;
};

typedef std::function<void(const HAEntity&)> StateChangeCallback;

struct StateChangedSubscription {
  String entity_id;
  std::vector<StateChangeCallback> callbacks;
  int message_id;  // Unique ID for the subscription
};

struct StateEventSubscriptionRequest {
  String entity_id;
};

class HomeAssistantClient {
private:
  WebsocketsClient client;
  String host;
  int port;
  String token;
  int message_id;
  bool connected;
  bool authenticated;
  unsigned long last_reconnect_attempt;
  unsigned long reconnect_interval;
  bool auto_reconnect_enabled;

  std::map<String, StateChangedSubscription> stateChangeCallbacks;
  std::map<int, String> subIdToEntityIdMap;

  void onMessageCallback(WebsocketsMessage message);
  void onEventCallback(WebsocketsEvent event, String data);
  void sendMessage(const JsonDocument& doc);
  void handleAuthResult(const JsonDocument& doc);
  void handleSubEvent(const JsonDocument& doc);
  void resubscribeToAllEvents();

public:
  HomeAssistantClient(const String& host, int port, const String& token);

  bool connect();
  void disconnect();
  void loop();
  bool isConnected() const;
  void reconnect();
  void setAutoReconnect(bool enabled);

  void subscribeToEvent(const String& entity_id, StateChangeCallback callback);

  void callService(const String& domain, const String& service, const String& entity_id,
                   const JsonObject& service_data = JsonObject());

  // Convenience methods for common actions
  void turnOnSwitch(const String& entity_id);
  void turnOffSwitch(const String& entity_id);
  void setLightBrightness(const String& entity_id, int brightness);
  void setInputNumber(const String& entity_id, float value);
};

#endif