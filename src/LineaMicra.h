
#ifndef LINEA_MICRA_H
#define LINEA_MICRA_H

#include "HomeAssistantClient.h"
#include "LaMarzoccoCloudClient.h"

// Structure to track pending state changes
struct PendingState {
  bool hasPending;
  unsigned long timestamp;

  PendingState() : hasPending(false), timestamp(0) {}
  void markPending() {
    hasPending = true;
    timestamp = millis();
  }
  void clearPending() {
    hasPending = false;
    timestamp = 0;
  }
  bool isPending() const {
    return hasPending;
  }
};

class LineaMicra {
public:
  // Constructor for dual-client mode (both Home Assistant and Cloud API)
  LineaMicra(HomeAssistantClient* haClient, LaMarzoccoCloudClient* cloudClient);

  ~LineaMicra();

  // State getters - return current values
  bool isOn();
  float getBoilerTemperature();
  float getPreBrewTime();
  float getPreBrewWait();
  bool isPreBrewOn();
  float getLastShotDuration();

  // Pending state checkers - for UI to show different colors
  bool isPowerPending() const;
  bool isBoilerTemperaturePending() const;
  bool isPreBrewModePending() const;
  bool isPreBrewTimesPending() const;

  // State setters with optimistic updates and debouncing
  bool setPower(bool enabled);
  bool setBoilerTemperature(float temperature);
  bool setPreBrewMode(bool enabled);
  bool setPreBrewTimes(float onTime, float waitTime);

  // Utility
  void loop();               // Call this in main loop to handle debouncing
  bool fetchInitialState();  // Fetch initial state from cloud API

private:
  HomeAssistantClient* haClient;
  LaMarzoccoCloudClient* cloudClient;

  // Current state values (updated by Home Assistant webhooks)
  volatile bool _isOn;
  float _boilerTemperature;
  bool _preBrewIsOn;
  float _preBrewTime;
  float _preBrewWait;
  float _lastShotDuration;

  // Pending state tracking for optimistic updates
  PendingState _powerPending;
  PendingState _boilerTempPending;
  PendingState _preBrewModePending;
  PendingState _preBrewTimesPending;

  // Debouncing configuration
  static const unsigned long DEBOUNCE_DELAY_MS = 1000;    // 1 second debounce
  static const unsigned long PENDING_TIMEOUT_MS = 10000;  // 10 second timeout

  // Pending API calls
  struct PendingApiCall {
    unsigned long scheduledTime;
    bool hasPendingCall;

    PendingApiCall() : scheduledTime(0), hasPendingCall(false) {}
    void schedule() {
      scheduledTime = millis() + DEBOUNCE_DELAY_MS;
      hasPendingCall = true;
    }
    void clear() {
      scheduledTime = 0;
      hasPendingCall = false;
    }
    bool isReady() const {
      return hasPendingCall && millis() >= scheduledTime;
    }
    bool isScheduled() const {
      return hasPendingCall;
    }
  };

  PendingApiCall _pendingPowerCall;
  bool _pendingPowerValue;

  PendingApiCall _pendingBoilerTempCall;
  float _pendingBoilerTempValue;

  PendingApiCall _pendingPreBrewModeCall;
  bool _pendingPreBrewModeValue;

  PendingApiCall _pendingPreBrewTimesCall;
  float _pendingPreBrewOnTime;
  float _pendingPreBrewWaitTime;

  void setupHomeAssistantSubscriptions();
  void processPendingApiCalls();
  void clearExpiredPendingStates();
};

#endif  // LINEA_MICRA_H