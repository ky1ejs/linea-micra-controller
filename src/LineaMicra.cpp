

#include "LineaMicra.h"

#include "config.h"

LineaMicra::LineaMicra(HomeAssistantClient* client)
    : haClient(client),
      cloudClient(nullptr),
      clientType(LineaMicraClientType::HOME_ASSISTANT),
      _isOn(false),
      _boilerTemperature(0.0f),
      _preBrewIsOn(false),
      _preBrewTime(0.0f),
      _preBrewWait(0.0f) {
  setupHomeAssistantSubscriptions();
}

LineaMicra::LineaMicra(LaMarzoccoCloudClient* client)
    : haClient(nullptr),
      cloudClient(client),
      clientType(LineaMicraClientType::CLOUD_API),
      _isOn(false),
      _boilerTemperature(0.0f),
      _preBrewIsOn(false),
      _preBrewTime(0.0f),
      _preBrewWait(0.0f) {
  // With cloud client, we'll pull state on demand rather than subscribe to events
  Serial.println("LineaMicra initialized with Cloud API client");
}

void LineaMicra::setupHomeAssistantSubscriptions() {
  haClient->subscribeToEvent(LINEA_MICRA_ENTITY_ID, [this](const HAEntity& entity) {
    // Handle the state change for Linea Micra
    if (entity.state == "on") {
      _isOn = true;
    } else if (entity.state == "off") {
      _isOn = false;
    }
    Serial.println("Linea Micra status updated: " + entity.state);
  });

  haClient->subscribeToEvent(LINEA_MICRA_BOILER_TEMP_ENTITY_ID, [this](const HAEntity& entity) {
    float newTemperature = entity.state.toFloat();
    // round to 0.1 degree precision
    newTemperature = round(newTemperature * 10.0f) / 10.0f;
    _boilerTemperature = newTemperature;
    Serial.println("Linea Micra target temperature updated: " + String(newTemperature));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_TIME_ENTITY_ID, [this](const HAEntity& entity) {
    _preBrewTime = entity.state.toFloat();
    Serial.println("Linea Micra pre-brew time updated: " + String(_preBrewTime));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_WAIT_ENTITY_ID, [this](const HAEntity& entity) {
    _preBrewWait = entity.state.toFloat();
    Serial.println("Linea Micra pre-brew wait time updated: " + String(_preBrewWait));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_MODE_ENTITY_ID, [this](const HAEntity& entity) {
    Serial.println("Linea Micra pre-brew infusion mode updated: " + entity.state);
    if (entity.state == "prebrew") {
      _preBrewIsOn = true;
    } else if (entity.state == "disabled") {
      _preBrewIsOn = false;
    }
  });
}

LineaMicra::~LineaMicra() {
  // Cleanup if necessary
  // Currently, no dynamic memory allocation is used, so nothing to free
}

bool LineaMicra::isOn() {
  if (clientType == LineaMicraClientType::CLOUD_API && cloudClient) {
    // For cloud API, refresh state on demand
    updateStateFromCloud();
  }
  return _isOn;
}

float LineaMicra::getBoilerTemperature() {
  if (clientType == LineaMicraClientType::CLOUD_API && cloudClient) {
    updateStateFromCloud();
  }
  return _boilerTemperature;
}

float LineaMicra::getPreBrewTime() {
  if (clientType == LineaMicraClientType::CLOUD_API && cloudClient) {
    updateStateFromCloud();
  }
  return _preBrewTime;
}

float LineaMicra::getPreBrewWait() {
  if (clientType == LineaMicraClientType::CLOUD_API && cloudClient) {
    updateStateFromCloud();
  }
  return _preBrewWait;
}

bool LineaMicra::isPreBrewOn() {
  if (clientType == LineaMicraClientType::CLOUD_API && cloudClient) {
    updateStateFromCloud();
  }
  return _preBrewIsOn;
}

bool LineaMicra::setPower(bool enabled) {
  if (clientType != LineaMicraClientType::CLOUD_API || !cloudClient) {
    Serial.println("setPower only available with Cloud API client");
    return false;
  }

  bool success = cloudClient->setPower(enabled);
  if (success) {
    _isOn = enabled;
    Serial.println("Machine power set to: " + String(enabled ? "ON" : "OFF"));
  }
  return success;
}

bool LineaMicra::setBoilerTemperature(float temperature) {
  if (clientType != LineaMicraClientType::CLOUD_API || !cloudClient) {
    Serial.println("setBoilerTemperature only available with Cloud API client");
    return false;
  }

  bool success = cloudClient->setBoilerTemperature(temperature);
  if (success) {
    _boilerTemperature = temperature;
    Serial.println("Boiler temperature set to: " + String(temperature));
  }
  return success;
}

bool LineaMicra::setPreBrewMode(bool enabled) {
  if (clientType != LineaMicraClientType::CLOUD_API || !cloudClient) {
    Serial.println("setPreBrewMode only available with Cloud API client");
    return false;
  }

  PreExtractionMode mode = enabled ? PreExtractionMode::PREBREWING : PreExtractionMode::DISABLED;
  bool success = cloudClient->setPreExtractionMode(mode);
  if (success) {
    _preBrewIsOn = enabled;
    Serial.println("Pre-brew mode set to: " + String(enabled ? "ENABLED" : "DISABLED"));
  }
  return success;
}

bool LineaMicra::setPreBrewTimes(float onTime, float waitTime) {
  if (clientType != LineaMicraClientType::CLOUD_API || !cloudClient) {
    Serial.println("setPreBrewTimes only available with Cloud API client");
    return false;
  }

  bool success = cloudClient->setPreExtractionTimes(onTime, waitTime);
  if (success) {
    _preBrewTime = onTime;
    _preBrewWait = waitTime;
    Serial.println("Pre-brew times set - On: " + String(onTime) + "s, Wait: " + String(waitTime) + "s");
  }
  return success;
}

bool LineaMicra::refreshFromCloud() {
  if (clientType != LineaMicraClientType::CLOUD_API || !cloudClient) {
    Serial.println("refreshFromCloud only available with Cloud API client");
    return false;
  }

  updateStateFromCloud();
  return true;
}

LineaMicraClientType LineaMicra::getClientType() const {
  return clientType;
}

void LineaMicra::updateStateFromCloud() {
  if (!cloudClient)
    return;

  MachineStatus status = cloudClient->getMachineStatus();
  if (status.isValid) {
    _isOn = status.isOn;
    _boilerTemperature = status.boilerTemperature;
    _preBrewIsOn = (status.preBrewMode != PreExtractionMode::DISABLED);
    _preBrewTime = status.preBrewTime;
    _preBrewWait = status.preBrewWait;

    Serial.println("State updated from cloud - On: " + String(_isOn) + ", Temp: " + String(_boilerTemperature) +
                   ", PreBrew: " + String(_preBrewIsOn));
  } else {
    Serial.println("Failed to get valid machine status from cloud");
  }
}
