#include "LaMarzoccoCloudClient.h"

LaMarzoccoCloudClient::LaMarzoccoCloudClient(const String& username, const String& password, const String& serialNumber)
    : username(username), password(password), serialNumber(serialNumber) {
    httpClient.setTimeout(15000); // 15 second timeout for cloud requests
}

LaMarzoccoCloudClient::~LaMarzoccoCloudClient() {
    // Cleanup handled by destructors
}

bool LaMarzoccoCloudClient::authenticate() {
    return signIn();
}

bool LaMarzoccoCloudClient::isAuthenticated() const {
    return accessToken.isValid();
}

bool LaMarzoccoCloudClient::signIn() {
    Serial.println("Signing in to LaMarzocco Cloud API...");
    
    JsonDocument authDoc;
    authDoc["username"] = username;
    authDoc["password"] = password;
    
    String url = String(CUSTOMER_APP_URL) + "/auth/signin";
    HTTPResponse response = httpClient.post(url, authDoc);
    
    if (!response.isSuccess()) {
        Serial.println("Authentication failed with status: " + String(response.statusCode));
        if (response.statusCode == 401) {
            Serial.println("Invalid username or password");
        }
        return false;
    }
    
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response.body);
    
    if (error) {
        Serial.println("Failed to parse authentication response: " + String(error.c_str()));
        return false;
    }
    
    accessToken.accessToken = responseDoc["access_token"].as<String>();
    accessToken.refreshToken = responseDoc["refresh_token"].as<String>();
    
    // Convert expires_in (seconds) to absolute timestamp (milliseconds)
    int expiresIn = responseDoc["expires_in"].as<int>();
    accessToken.expiresAt = millis() + (expiresIn * 1000);
    
    Serial.println("Successfully authenticated with LaMarzocco Cloud API");
    return true;
}

bool LaMarzoccoCloudClient::refreshToken() {
    Serial.println("Refreshing LaMarzocco Cloud API token...");
    
    JsonDocument refreshDoc;
    refreshDoc["username"] = username;
    refreshDoc["refresh_token"] = accessToken.refreshToken;
    
    String url = String(CUSTOMER_APP_URL) + "/auth/refreshtoken";
    HTTPResponse response = httpClient.post(url, refreshDoc);
    
    if (!response.isSuccess()) {
        Serial.println("Token refresh failed with status: " + String(response.statusCode));
        return signIn(); // Fall back to full sign in
    }
    
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response.body);
    
    if (error) {
        Serial.println("Failed to parse token refresh response: " + String(error.c_str()));
        return signIn(); // Fall back to full sign in
    }
    
    accessToken.accessToken = responseDoc["access_token"].as<String>();
    accessToken.refreshToken = responseDoc["refresh_token"].as<String>();
    
    int expiresIn = responseDoc["expires_in"].as<int>();
    accessToken.expiresAt = millis() + (expiresIn * 1000);
    
    Serial.println("Successfully refreshed LaMarzocco Cloud API token");
    return true;
}

bool LaMarzoccoCloudClient::ensureValidToken() {
    if (!accessToken.isValid()) {
        return signIn();
    }
    
    if (accessToken.needsRefresh()) {
        return refreshToken();
    }
    
    return true;
}

MachineStatus LaMarzoccoCloudClient::getMachineStatus() {
    MachineStatus status;
    
    if (!ensureValidToken()) {
        Serial.println("Failed to ensure valid token for getMachineStatus");
        return status;
    }
    
    httpClient.setDefaultHeader("Authorization", "Bearer " + accessToken.accessToken);
    
    String url = String(CUSTOMER_APP_URL) + "/things/" + serialNumber + "/dashboard";
    HTTPResponse response = httpClient.get(url);
    
    if (!response.isSuccess()) {
        Serial.println("Failed to get machine dashboard with status: " + String(response.statusCode));
        return status;
    }
    
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response.body);
    
    if (error) {
        Serial.println("Failed to parse dashboard response: " + String(error.c_str()));
        return status;
    }
    
    // Parse the dashboard response
    JsonArray widgets = responseDoc["widgets"];
    for (JsonObject widget : widgets) {
        String widgetType = widget["widgetType"].as<String>();
        
        if (widgetType == "CMMachineStatus") {
            String machineState = widget["state"].as<String>();
            status.isOn = (machineState == "PoweredOn" || machineState == "Brewing");
        }
        else if (widgetType == "CMCoffeeBoiler") {
            status.boilerTemperature = widget["targetTemperature"].as<float>();
        }
        else if (widgetType == "CMPreBrewing") {
            String mode = widget["mode"].as<String>();
            status.preBrewMode = stringToPreExtractionMode(mode);
            status.preBrewTime = widget["preBrewingTime"].as<float>();
            status.preBrewWait = widget["preBrewingPauseTime"].as<float>();
        }
    }
    
    status.isValid = true;
    return status;
}

bool LaMarzoccoCloudClient::setPower(bool enabled) {
    JsonDocument commandData;
    commandData["mode"] = enabled ? "BrewingMode" : "StandBy";
    
    return executeCommand("CoffeeMachineChangeMode", commandData);
}

bool LaMarzoccoCloudClient::setBoilerTemperature(float temperature) {
    JsonDocument commandData;
    commandData["boilerIndex"] = 1;
    commandData["targetTemperature"] = round(temperature * 10.0f) / 10.0f; // Round to 0.1 precision
    
    return executeCommand("CoffeeMachineSettingCoffeeBoilerTargetTemperature", commandData);
}

bool LaMarzoccoCloudClient::setPreExtractionMode(PreExtractionMode mode) {
    JsonDocument commandData;
    commandData["mode"] = preExtractionModeToString(mode);
    
    return executeCommand("CoffeeMachinePreBrewingChangeMode", commandData);
}

bool LaMarzoccoCloudClient::setPreExtractionTimes(float preBrewTime, float preBrewWait) {
    JsonDocument commandData;
    commandData["preBrewingTime"] = preBrewTime;
    commandData["preBrewingPauseTime"] = preBrewWait;
    
    return executeCommand("CoffeeMachinePreBrewingSettingTimes", commandData);
}

bool LaMarzoccoCloudClient::executeCommand(const String& command, const JsonDocument& data) {
    if (!ensureValidToken()) {
        Serial.println("Failed to ensure valid token for command: " + command);
        return false;
    }
    
    httpClient.setDefaultHeader("Authorization", "Bearer " + accessToken.accessToken);
    
    String url = String(CUSTOMER_APP_URL) + "/things/" + serialNumber + "/command/" + command;
    HTTPResponse response = httpClient.post(url, data);
    
    if (!response.isSuccess()) {
        Serial.println("Command " + command + " failed with status: " + String(response.statusCode));
        Serial.println("Response: " + response.body);
        return false;
    }
    
    Serial.println("Command " + command + " executed successfully");
    return true;
}

String LaMarzoccoCloudClient::preExtractionModeToString(PreExtractionMode mode) {
    switch (mode) {
        case PreExtractionMode::PREINFUSION: return "PreInfusion";
        case PreExtractionMode::PREBREWING: return "PreBrewing";
        case PreExtractionMode::DISABLED: return "Disabled";
        default: return "Disabled";
    }
}

PreExtractionMode LaMarzoccoCloudClient::stringToPreExtractionMode(const String& mode) {
    if (mode == "PreInfusion") return PreExtractionMode::PREINFUSION;
    if (mode == "PreBrewing") return PreExtractionMode::PREBREWING;
    return PreExtractionMode::DISABLED;
}

void LaMarzoccoCloudClient::setSerialNumber(const String& serial) {
    serialNumber = serial;
}

String LaMarzoccoCloudClient::getSerialNumber() const {
    return serialNumber;
}