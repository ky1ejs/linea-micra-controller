#include "HomeAssistantClient.h"

HomeAssistantClient::HomeAssistantClient(const String& host, int port, const String& token)
    : host(host), port(port), token(token), message_id(1), connected(false), authenticated(false) {
    
    client.onMessage([this](WebsocketsMessage message) {
        this->onMessageCallback(message);
    });
    
    client.onEvent([this](WebsocketsEvent event, String data) {
        this->onEventCallback(event, data);
    });
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
    
    String type = doc["type"];
    
    if (type == "auth_required") {
        JsonDocument auth_msg;
        auth_msg["type"] = "auth";
        auth_msg["access_token"] = token;
        sendMessage(auth_msg);
        Serial.println("Sent authentication token");
    }
    else if (type == "auth_ok") {
        handleAuthResult(doc);
    }
    else if (type == "auth_invalid") {
        Serial.println("Authentication failed - invalid token");
        disconnect();
    }
    else if (type == "event") {
        handleStateChanged(doc);
    }
    else if (type == "result") {
        handleResult(doc);
    }
}

void HomeAssistantClient::onEventCallback(WebsocketsEvent event, String data) {
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
    
    // Automatically subscribe to state changes after authentication
    subscribeToEvents();
}

void HomeAssistantClient::handleStateChanged(const JsonDocument& doc) {
    if (state_callback && doc["event"]["event_type"] == "state_changed") {
        JsonObjectConst event = doc["event"];
        JsonObjectConst data = event["data"];
        JsonObjectConst new_state = data["new_state"];
        
        HAEntity entity;
        entity.entity_id = new_state["entity_id"].as<String>();
        entity.state = new_state["state"].as<String>();
        
        if (new_state["attributes"]["friendly_name"]) {
            entity.friendly_name = new_state["attributes"]["friendly_name"].as<String>();
        }
        
        entity.last_updated = millis();
        state_callback(entity);
    }
}

void HomeAssistantClient::handleResult(const JsonDocument& doc) {
    bool success = doc["success"];
    int id = doc["id"];
    
    if (service_callback) {
        String message = success ? "Success" : "Failed";
        if (doc["error"]) {
            message = doc["error"]["message"].as<String>();
        }
        service_callback(success, message);
    }
    
    Serial.println("Service call " + String(id) + ": " + (success ? "Success" : "Failed"));
}

void HomeAssistantClient::setStateChangeCallback(StateChangeCallback callback) {
    state_callback = callback;
}

void HomeAssistantClient::setServiceCallCallback(ServiceCallCallback callback) {
    service_callback = callback;
}

void HomeAssistantClient::subscribeToEvents() {
    JsonDocument sub_msg;
    sub_msg["id"] = message_id++;
    sub_msg["type"] = "subscribe_events";
    sub_msg["event_type"] = "state_changed";
    
    sendMessage(sub_msg);
    Serial.println("Subscribed to state change events");
}

void HomeAssistantClient::callService(const String& domain, const String& service, const String& entity_id, const JsonObject& service_data) {
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