#ifndef HOME_ASSISTANT_CLIENT_H
#define HOME_ASSISTANT_CLIENT_H

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <WiFi.h>

using namespace websockets;

struct HAEntity {
    String entity_id;
    String state;
    String friendly_name;
    unsigned long last_updated;
};

typedef std::function<void(const HAEntity&)> StateChangeCallback;
typedef std::function<void(bool, const String&)> ServiceCallCallback;

class HomeAssistantClient {
private:
    WebsocketsClient client;
    String host;
    int port;
    String token;
    int message_id;
    bool connected;
    bool authenticated;
    
    StateChangeCallback state_callback;
    ServiceCallCallback service_callback;
    
    void onMessageCallback(WebsocketsMessage message);
    void onEventCallback(WebsocketsEvent event, String data);
    void sendMessage(const JsonDocument& doc);
    void handleAuthResult(const JsonDocument& doc);
    void handleStateChanged(const JsonDocument& doc);
    void handleResult(const JsonDocument& doc);
    
public:
    HomeAssistantClient(const String& host, int port, const String& token);
    
    bool connect();
    void disconnect();
    void loop();
    bool isConnected() const;
    
    void setStateChangeCallback(StateChangeCallback callback);
    void setServiceCallCallback(ServiceCallCallback callback);
    
    void subscribeToEvents();
    void callService(const String& domain, const String& service, const String& entity_id, const JsonObject& service_data = JsonObject());
    
    // Convenience methods for common actions
    void turnOnSwitch(const String& entity_id);
    void turnOffSwitch(const String& entity_id);
    void setLightBrightness(const String& entity_id, int brightness);
    void setInputNumber(const String& entity_id, float value);
};

#endif