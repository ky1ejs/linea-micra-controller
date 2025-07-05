#include "HomeAssistantClient.h"

HomeAssistantClient::HomeAssistantClient(const String& host, int port, const String& token)
    : host(host),
      port(port),
      token(token),
      message_id(1),
      connected(false),
      authenticated(false),
      last_reconnect_attempt(0),
      reconnect_interval(5000),  // 5 seconds between reconnect attempts
      auto_reconnect_enabled(true),
      stateChangeCallbacks(),
      subIdToEntityIdMap() {
  client.onMessage([this](WebsocketsMessage message) { this->onMessageCallback(message); });

  client.onEvent([this](WebsocketsEvent event, String data) { this->onEventCallback(event, data); });
}

bool HomeAssistantClient::connect() {
  String url = "ws://" + host + ":" + String(port) + "/api/websocket";
  Serial.println("Connecting to: " + url);

  connected = client.connect(url);
  if (!connected) {
    Serial.println("Failed to connect to Home Assistant WebSocket");
    return false;
  }

  Serial.println("WebSocket connected, waiting for auth_required message...");
  return true;
}

void HomeAssistantClient::disconnect() {
  client.close();
  connected = false;
  authenticated = false;
}

void HomeAssistantClient::loop() {
  if (connected) {
    client.poll();
  } else if (auto_reconnect_enabled && (millis() - last_reconnect_attempt) > reconnect_interval) {
    Serial.println("Attempting to reconnect to Home Assistant...");
    reconnect();
  }
}

bool HomeAssistantClient::isConnected() const {
  return connected && authenticated;
}

void HomeAssistantClient::onMessageCallback(WebsocketsMessage message) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message.data());

  if (error) {
    Serial.println("Failed to parse JSON message");
    return;
  }

  Serial.println("Received message: " + message.data());

  String type = doc["type"];

  if (type == "auth_required") {
    JsonDocument auth_msg;
    auth_msg["type"] = "auth";
    auth_msg["access_token"] = token;
    sendMessage(auth_msg);
    Serial.println("Sent authentication token");
  } else if (type == "auth_ok") {
    handleAuthResult(doc);
  } else if (type == "auth_invalid") {
    Serial.println("Authentication failed - invalid token");
    disconnect();
  } else if (type == "event") {
    handleSubEvent(doc);
  } else if (type == "result") {
    Serial.println("Received result message");
  }
}

void HomeAssistantClient::onEventCallback(WebsocketsEvent event, String data) {
  Serial.println("WebSocket event: " + String(static_cast<int>(event)) + " - " + data);
  switch (event) {
    case WebsocketsEvent::ConnectionOpened:
      Serial.println("WebSocket connection opened");
      break;
    case WebsocketsEvent::ConnectionClosed:
      Serial.println("WebSocket connection closed");
      connected = false;
      authenticated = false;
      break;
    case WebsocketsEvent::GotPing:
      Serial.println("Got ping");
      break;
    case WebsocketsEvent::GotPong:
      Serial.println("Got pong");
      break;
  }
}

void HomeAssistantClient::sendMessage(const JsonDocument& doc) {
  String message;
  serializeJson(doc, message);
  client.send(message);
}

void HomeAssistantClient::handleAuthResult(const JsonDocument& doc) {
  authenticated = true;
  Serial.println("Successfully authenticated with Home Assistant");
}

void HomeAssistantClient::handleSubEvent(const JsonDocument& doc) {
  auto data = doc["event"]["variables"]["trigger"]["to_state"];
  String entity_id = data["entity_id"].as<String>();
  auto sub = stateChangeCallbacks.find(entity_id);

  Serial.println("Handling state change event for entity: " + entity_id);

  if (sub == stateChangeCallbacks.end()) {
    Serial.println("No subscription found for entity: " + entity_id);
    return;
  }

  auto& subEntry = sub->second;
  for (const auto& callback : subEntry.callbacks) {
    HAEntity entity;
    entity.entity_id = entity_id;
    entity.state = data["state"].as<String>();
    if (data["attributes"]["friendly_name"]) {
      entity.friendly_name = data["attributes"]["friendly_name"].as<String>();
    }

    callback(entity);
  }
}

int nextMessageId() {
  static int message_id = 1;
  return message_id++;
}

void HomeAssistantClient::subscribeToEvent(const String& entity_id, StateChangeCallback callback) {
  try {
    auto subscription = stateChangeCallbacks.at(entity_id);
    subscription.callbacks.push_back(callback);
    Serial.println("Subscribed to entity: " + entity_id);
  } catch (const std::out_of_range&) {
    // Subscription not found, create a new one
    int id = nextMessageId();
    JsonDocument msg;
    msg["id"] = id;
    msg["type"] = "subscribe_trigger";

    JsonObject trigger = msg["trigger"].to<JsonObject>();
    trigger["platform"] = "state";
    trigger["entity_id"] = entity_id;

    sendMessage(msg);

    stateChangeCallbacks[entity_id] = StateChangedSubscription{
        entity_id,
        {callback},
        id  // Store the message ID for this subscription
    };
    subIdToEntityIdMap[id] = entity_id;
    Serial.println("Subscribed to state change events for entity: " + entity_id);
  }
}

void HomeAssistantClient::callService(const String& domain, const String& service, const String& entity_id,
                                      const JsonObject& service_data) {
  JsonDocument msg;
  msg["id"] = message_id++;
  msg["type"] = "call_service";
  msg["domain"] = domain;
  msg["service"] = service;

  JsonObject target = msg["target"].to<JsonObject>();
  target["entity_id"] = entity_id;

  if (!service_data.isNull()) {
    msg["service_data"] = service_data;
  }

  sendMessage(msg);
  Serial.println("Called service: " + domain + "." + service + " on " + entity_id);
}

void HomeAssistantClient::turnOnSwitch(const String& entity_id) {
  callService("homeassistant", "turn_on", entity_id);
}

void HomeAssistantClient::turnOffSwitch(const String& entity_id) {
  callService("homeassistant", "turn_off", entity_id);
}

void HomeAssistantClient::setLightBrightness(const String& entity_id, int brightness) {
  JsonDocument service_data;
  service_data["brightness"] = brightness;
  callService("light", "turn_on", entity_id, service_data.template as<JsonObject>());
}

void HomeAssistantClient::setInputNumber(const String& entity_id, float value) {
  JsonDocument service_data;
  service_data["value"] = value;
  callService("input_number", "set_value", entity_id, service_data.template as<JsonObject>());
}

void HomeAssistantClient::reconnect() {
  last_reconnect_attempt = millis();

  if (connected) {
    disconnect();
  }

  if (connect()) {
    Serial.println("HomeAssistant reconnection successful, re-establishing subscriptions...");
    // Re-establish all subscriptions after successful reconnection
    resubscribeToAllEvents();
  } else {
    Serial.println("HomeAssistant reconnection failed, will retry later");
  }
}

void HomeAssistantClient::setAutoReconnect(bool enabled) {
  auto_reconnect_enabled = enabled;
  if (enabled) {
    Serial.println("HomeAssistant auto-reconnect enabled");
  } else {
    Serial.println("HomeAssistant auto-reconnect disabled");
  }
}

void HomeAssistantClient::resubscribeToAllEvents() {
  if (!isConnected()) {
    Serial.println("Cannot resubscribe: not connected to HomeAssistant");
    return;
  }

  Serial.println("Re-establishing " + String(stateChangeCallbacks.size()) + " HomeAssistant subscriptions...");

  // Clear the message ID mappings since we'll be creating new subscriptions
  subIdToEntityIdMap.clear();

  // Re-subscribe to all entities by sending new subscription messages
  for (auto& subscription : stateChangeCallbacks) {
    const String& entity_id = subscription.first;
    StateChangedSubscription& sub = subscription.second;

    // Create new subscription with new message ID
    int id = nextMessageId();
    JsonDocument msg;
    msg["id"] = id;
    msg["type"] = "subscribe_trigger";

    JsonObject trigger = msg["trigger"].to<JsonObject>();
    trigger["platform"] = "state";
    trigger["entity_id"] = entity_id;

    sendMessage(msg);

    // Update the subscription with the new message ID
    sub.message_id = id;
    subIdToEntityIdMap[id] = entity_id;

    Serial.println("Re-subscribed to entity: " + entity_id);
  }

  Serial.println("All HomeAssistant subscriptions re-established");
}