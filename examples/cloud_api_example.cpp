//
// Example: Using LaMarzocco Cloud API directly
//
// This example shows how to use the new cloud API integration
// to control your Linea Micra coffee machine directly.
//

#include <Arduino.h>
#include "WiFiManager.h"
#include "LaMarzoccoCloudClient.h"
#include "LineaMicra.h"
#include "config.h"

// Initialize clients
WiFiManager wifiManager(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT);
LaMarzoccoCloudClient cloudClient(LM_USERNAME, LM_PASSWORD, LM_SERIAL_NUMBER);
LineaMicra *lineaMicra = nullptr;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (!wifiManager.connect()) {
        Serial.println("Failed to connect to WiFi");
        while (1) delay(1000);
    }
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    
    // Authenticate with LaMarzocco Cloud
    Serial.println("Authenticating with LaMarzocco Cloud...");
    if (!cloudClient.authenticate()) {
        Serial.println("Failed to authenticate with LaMarzocco Cloud");
        while (1) delay(1000);
    }
    Serial.println("Successfully authenticated with LaMarzocco Cloud");
    
    // Initialize LineaMicra with cloud client
    lineaMicra = new LineaMicra(&cloudClient);
    Serial.println("LineaMicra initialized with Cloud API");
    
    // Demonstrate reading current state
    Serial.println("\n=== Current Machine State ===");
    Serial.println("Power: " + String(lineaMicra->isOn() ? "ON" : "OFF"));
    Serial.println("Boiler Temperature: " + String(lineaMicra->getBoilerTemperature()) + "°C");
    Serial.println("Pre-brew Enabled: " + String(lineaMicra->isPreBrewOn() ? "YES" : "NO"));
    Serial.println("Pre-brew Time: " + String(lineaMicra->getPreBrewTime()) + "s");
    Serial.println("Pre-brew Wait: " + String(lineaMicra->getPreBrewWait()) + "s");
    
    // Demonstrate machine control
    Serial.println("\n=== Demonstrating Machine Control ===");
    
    // Turn machine on
    Serial.println("Turning machine ON...");
    if (lineaMicra->setPower(true)) {
        Serial.println("✓ Machine turned ON successfully");
    } else {
        Serial.println("✗ Failed to turn machine ON");
    }
    
    delay(2000);
    
    // Set boiler temperature
    Serial.println("Setting boiler temperature to 93.0°C...");
    if (lineaMicra->setBoilerTemperature(93.0)) {
        Serial.println("✓ Boiler temperature set successfully");
    } else {
        Serial.println("✗ Failed to set boiler temperature");
    }
    
    delay(2000);
    
    // Enable pre-brew
    Serial.println("Enabling pre-brew mode...");
    if (lineaMicra->setPreBrewMode(true)) {
        Serial.println("✓ Pre-brew mode enabled successfully");
    } else {
        Serial.println("✗ Failed to enable pre-brew mode");
    }
    
    delay(2000);
    
    // Set pre-brew times
    Serial.println("Setting pre-brew times (2.0s on, 1.5s wait)...");
    if (lineaMicra->setPreBrewTimes(2.0, 1.5)) {
        Serial.println("✓ Pre-brew times set successfully");
    } else {
        Serial.println("✗ Failed to set pre-brew times");
    }
    
    Serial.println("\n=== Setup Complete ===");
    Serial.println("Machine is now configured and ready!");
}

void loop() {
    // Handle WiFi
    wifiManager.loop();
    
    // Refresh machine state every 10 seconds
    static unsigned long lastRefresh = 0;
    if (millis() - lastRefresh > 10000) {
        lastRefresh = millis();
        
        if (lineaMicra) {
            lineaMicra->refreshFromCloud();
            
            // Print current state
            Serial.println("\n--- Current State ---");
            Serial.println("Power: " + String(lineaMicra->isOn() ? "ON" : "OFF"));
            Serial.println("Temperature: " + String(lineaMicra->getBoilerTemperature()) + "°C");
            Serial.println("Pre-brew: " + String(lineaMicra->isPreBrewOn() ? "ON" : "OFF"));
        }
    }
    
    delay(100);
}

//
// Alternative: Manual control example
//
void manualControlExample() {
    // You can also control the machine directly via the cloud client
    
    // Turn machine on
    cloudClient.setPower(true);
    
    // Set temperature to 92.5°C
    cloudClient.setBoilerTemperature(92.5);
    
    // Enable pre-brewing mode
    cloudClient.setPreExtractionMode(PreExtractionMode::PREBREWING);
    
    // Set pre-brew timing: 2.5s on, 2.0s wait
    cloudClient.setPreExtractionTimes(2.5, 2.0);
    
    // Get current machine status
    MachineStatus status = cloudClient.getMachineStatus();
    if (status.isValid) {
        Serial.println("Machine is " + String(status.isOn ? "ON" : "OFF"));
        Serial.println("Boiler at " + String(status.boilerTemperature) + "°C");
    }
}