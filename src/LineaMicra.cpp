

#include "LineaMicra.h"

#include "config.h"

LineaMicra::LineaMicra(HomeAssistantClient* haClient, LaMarzoccoCloudClient* cloudClient)
    : haClient(haClient),
      cloudClient(cloudClient),
      _isOn(false),
      _boilerTemperature(0.0f),
      _preBrewIsOn(false),
      _preBrewTime(0.0f),
      _preBrewWait(0.0f),
      _pendingPowerValue(false),
      _pendingBoilerTempValue(0.0f),
      _pendingPreBrewModeValue(false),
      _pendingPreBrewOnTime(0.0f),
      _pendingPreBrewWaitTime(0.0f) {
  // Setup Home Assistant subscriptions for real-time updates
  setupHomeAssistantSubscriptions();

  Serial.println("LineaMicra initialized with dual-client mode (HA + Cloud API)");
}

void LineaMicra::setupHomeAssistantSubscriptions() {
  if (!haClient)
    return;

  haClient->subscribeToEvent(LINEA_MICRA_ENTITY_ID, [this](const HAEntity& entity) {
    // Handle the state change for Linea Micra
    if (entity.state == "on") {
      _isOn = true;
    } else if (entity.state == "off") {
      _isOn = false;
    }
    // Clear pending state when we get confirmation from HA
    _powerPending.clearPending();
    Serial.println("Linea Micra status updated: " + entity.state);
  });

  haClient->subscribeToEvent(LINEA_MICRA_BOILER_TEMP_ENTITY_ID, [this](const HAEntity& entity) {
    float newTemperature = entity.state.toFloat();
    // round to 0.1 degree precision
    newTemperature = round(newTemperature * 10.0f) / 10.0f;
    _boilerTemperature = newTemperature;
    // Clear pending state when we get confirmation from HA
    _boilerTempPending.clearPending();
    Serial.println("Linea Micra target temperature updated: " + String(newTemperature));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_TIME_ENTITY_ID, [this](const HAEntity& entity) {
    _preBrewTime = entity.state.toFloat();
    // Clear pending state when we get confirmation from HA
    _preBrewTimesPending.clearPending();
    Serial.println("Linea Micra pre-brew time updated: " + String(_preBrewTime));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_WAIT_ENTITY_ID, [this](const HAEntity& entity) {
    _preBrewWait = entity.state.toFloat();
    // Clear pending state when we get confirmation from HA
    _preBrewTimesPending.clearPending();
    Serial.println("Linea Micra pre-brew wait time updated: " + String(_preBrewWait));
  });

  haClient->subscribeToEvent(LINEA_MICRA_PREBREW_MODE_ENTITY_ID, [this](const HAEntity& entity) {
    Serial.println("Linea Micra pre-brew infusion mode updated: " + entity.state);
    if (entity.state == "prebrew") {
      _preBrewIsOn = true;
    } else if (entity.state == "disabled") {
      _preBrewIsOn = false;
    }
    // Clear pending state when we get confirmation from HA
    _preBrewModePending.clearPending();
  });
}

LineaMicra::~LineaMicra() {
  // Cleanup if necessary
  // Currently, no dynamic memory allocation is used, so nothing to free
}

// State getters - return current values
bool LineaMicra::isOn() {
  return _isOn;
}

float LineaMicra::getBoilerTemperature() {
  return _boilerTemperature;
}

float LineaMicra::getPreBrewTime() {
  return _preBrewTime;
}

float LineaMicra::getPreBrewWait() {
  return _preBrewWait;
}

bool LineaMicra::isPreBrewOn() {
  return _preBrewIsOn;
}

// Pending state checkers for UI
bool LineaMicra::isPowerPending() const {
  return _powerPending.isPending();
}

bool LineaMicra::isBoilerTemperaturePending() const {
  return _boilerTempPending.isPending();
}

bool LineaMicra::isPreBrewModePending() const {
  return _preBrewModePending.isPending();
}

bool LineaMicra::isPreBrewTimesPending() const {
  return _preBrewTimesPending.isPending();
}

// State setters with optimistic updates and debouncing
bool LineaMicra::setPower(bool enabled) {
  if (!cloudClient) {
    Serial.println("setPower requires Cloud API client");
    return false;
  }

  // Optimistic update
  _isOn = enabled;
  _powerPending.markPending();

  // Schedule API call (with debouncing)
  _pendingPowerValue = enabled;
  _pendingPowerCall.schedule();

  Serial.println("Power optimistically set to: " + String(enabled ? "ON" : "OFF") + " (pending API call)");
  return true;
}

bool LineaMicra::setBoilerTemperature(float temperature) {
  if (!cloudClient) {
    Serial.println("setBoilerTemperature requires Cloud API client");
    return false;
  }

  // Optimistic update
  _boilerTemperature = temperature;
  _boilerTempPending.markPending();

  // Schedule API call (with debouncing)
  _pendingBoilerTempValue = temperature;
  _pendingBoilerTempCall.schedule();

  Serial.println("Boiler temperature optimistically set to: " + String(temperature) + " (pending API call)");
  return true;
}

bool LineaMicra::setPreBrewMode(bool enabled) {
  if (!cloudClient) {
    Serial.println("setPreBrewMode requires Cloud API client");
    return false;
  }

  // Optimistic update
  _preBrewIsOn = enabled;
  _preBrewModePending.markPending();

  // Schedule API call (with debouncing)
  _pendingPreBrewModeValue = enabled;
  _pendingPreBrewModeCall.schedule();

  Serial.println("Pre-brew mode optimistically set to: " + String(enabled ? "ENABLED" : "DISABLED") +
                 " (pending API call)");
  return true;
}

bool LineaMicra::setPreBrewTimes(float onTime, float waitTime) {
  if (!cloudClient) {
    Serial.println("setPreBrewTimes requires Cloud API client");
    return false;
  }

  // Optimistic update
  _preBrewTime = onTime;
  _preBrewWait = waitTime;
  _preBrewTimesPending.markPending();

  // Schedule API call (with debouncing)
  _pendingPreBrewOnTime = onTime;
  _pendingPreBrewWaitTime = waitTime;
  _pendingPreBrewTimesCall.schedule();

  Serial.println("Pre-brew times optimistically set - On: " + String(onTime) + "s, Wait: " + String(waitTime) +
                 "s (pending API call)");
  return true;
}

// Fetch initial state from cloud API
bool LineaMicra::fetchInitialState() {
  if (!cloudClient) {
    Serial.println("fetchInitialState requires Cloud API client");
    return false;
  }

  MachineStatus status = cloudClient->getMachineStatus();
  if (status.isValid) {
    _isOn = status.isOn;
    _boilerTemperature = status.boilerTemperature;
    _preBrewIsOn = (status.preBrewMode != PreExtractionMode::NONE);
    _preBrewTime = status.preBrewTime;
    _preBrewWait = status.preBrewWait;

    Serial.println("Initial state fetched from cloud - On: " + String(_isOn) + ", Temp: " + String(_boilerTemperature) +
                   ", PreBrew: " + String(_preBrewIsOn));
    return true;
  } else {
    Serial.println("Failed to fetch initial state from cloud");
    return false;
  }
}

// Main loop function to handle debouncing and API calls
void LineaMicra::loop() {
  processPendingApiCalls();
  clearExpiredPendingStates();
}

void LineaMicra::processPendingApiCalls() {
  if (!cloudClient)
    return;

  // Process power API call
  if (_pendingPowerCall.isReady()) {
    Serial.println("Executing pending power API call: " + String(_pendingPowerValue ? "ON" : "OFF"));
    bool success = cloudClient->setPower(_pendingPowerValue);
    _pendingPowerCall.clear();

    if (!success) {
      Serial.println("Power API call failed, reverting optimistic update");
      // Revert optimistic update on failure
      _isOn = !_pendingPowerValue;
      _powerPending.clearPending();
    }
  }

  // Process boiler temperature API call
  if (_pendingBoilerTempCall.isReady()) {
    Serial.println("Executing pending boiler temp API call: " + String(_pendingBoilerTempValue));
    bool success = cloudClient->setBoilerTemperature(_pendingBoilerTempValue);
    _pendingBoilerTempCall.clear();

    if (!success) {
      Serial.println("Boiler temp API call failed, pending state will timeout");
      // Don't revert temperature immediately, let it timeout or be corrected by HA
    }
  }

  // Process pre-brew mode API call
  if (_pendingPreBrewModeCall.isReady()) {
    Serial.println("Executing pending pre-brew mode API call: " +
                   String(_pendingPreBrewModeValue ? "ENABLED" : "DISABLED"));
    PreExtractionMode mode = _pendingPreBrewModeValue ? PreExtractionMode::PREBREWING : PreExtractionMode::NONE;
    bool success = cloudClient->setPreExtractionMode(mode);
    _pendingPreBrewModeCall.clear();

    if (!success) {
      Serial.println("Pre-brew mode API call failed, reverting optimistic update");
      _preBrewIsOn = !_pendingPreBrewModeValue;
      _preBrewModePending.clearPending();
    }
  }

  // Process pre-brew times API call
  if (_pendingPreBrewTimesCall.isReady()) {
    Serial.println("Executing pending pre-brew times API call: On=" + String(_pendingPreBrewOnTime) +
                   "s, Wait=" + String(_pendingPreBrewWaitTime) + "s");
    bool success = cloudClient->setPreExtractionTimes(_pendingPreBrewOnTime, _pendingPreBrewWaitTime);
    _pendingPreBrewTimesCall.clear();

    if (!success) {
      Serial.println("Pre-brew times API call failed, pending state will timeout");
      // Don't revert times immediately, let them timeout or be corrected by HA
    }
  }
}

void LineaMicra::clearExpiredPendingStates() {
  unsigned long now = millis();

  // Clear expired pending states (in case HA webhook never comes)
  if (_powerPending.isPending() && (now - _powerPending.timestamp) > PENDING_TIMEOUT_MS) {
    Serial.println("Power pending state expired, clearing");
    _powerPending.clearPending();
  }

  if (_boilerTempPending.isPending() && (now - _boilerTempPending.timestamp) > PENDING_TIMEOUT_MS) {
    Serial.println("Boiler temp pending state expired, clearing");
    _boilerTempPending.clearPending();
  }

  if (_preBrewModePending.isPending() && (now - _preBrewModePending.timestamp) > PENDING_TIMEOUT_MS) {
    Serial.println("Pre-brew mode pending state expired, clearing");
    _preBrewModePending.clearPending();
  }

  if (_preBrewTimesPending.isPending() && (now - _preBrewTimesPending.timestamp) > PENDING_TIMEOUT_MS) {
    Serial.println("Pre-brew times pending state expired, clearing");
    _preBrewTimesPending.clearPending();
  }
}
