#ifndef LAMARZOCCO_CLOUD_CLIENT_H
#define LAMARZOCCO_CLOUD_CLIENT_H

#include <ArduinoJson.h>

#include "HTTPClientWrapper.h"

// Constants
#define BASE_URL "lion.lamarzocco.io"
#define CUSTOMER_APP_URL "https://" BASE_URL "/api/customer-app"

// Token refresh time (10 minutes before expiration in milliseconds)
#define TOKEN_TIME_TO_REFRESH (10 * 60 * 1000)

enum class PreExtractionMode { PREINFUSION, PREBREWING, NONE };

enum class MachineMode { BREWING_MODE, STANDBY };

struct AccessToken {
  String accessToken;
  String refreshToken;
  unsigned long expiresAt;  // timestamp in milliseconds

  bool isValid() const {
    return accessToken.length() > 0 && expiresAt > millis();
  }

  bool needsRefresh() const {
    return expiresAt < (millis() + TOKEN_TIME_TO_REFRESH);
  }
};

struct MachineStatus {
  bool isOn;
  float boilerTemperature;
  PreExtractionMode preBrewMode;
  float preBrewTime;
  float preBrewWait;
  bool isValid;

  MachineStatus()
      : isOn(false),
        boilerTemperature(0.0f),
        preBrewMode(PreExtractionMode::NONE),
        preBrewTime(0.0f),
        preBrewWait(0.0f),
        isValid(false) {}
};

class LaMarzoccoCloudClient {
private:
  HTTPClientWrapper httpClient;
  String username;
  String password;
  String serialNumber;
  AccessToken accessToken;

  bool signIn();
  bool refreshToken();
  bool ensureValidToken();
  String preExtractionModeToString(PreExtractionMode mode);
  PreExtractionMode stringToPreExtractionMode(const String& mode);
  bool executeCommand(const String& command, const JsonDocument& data);

public:
  LaMarzoccoCloudClient(const String& username, const String& password, const String& serialNumber);
  ~LaMarzoccoCloudClient();

  // Authentication
  bool authenticate();
  bool isAuthenticated() const;

  // Machine state retrieval
  MachineStatus getMachineStatus();

  // Machine control
  bool setPower(bool enabled);
  bool setBoilerTemperature(float temperature);
  bool setPreExtractionMode(PreExtractionMode mode);
  bool setPreExtractionTimes(float preBrewTime, float preBrewWait);

  // Utility
  void setSerialNumber(const String& serial);
  String getSerialNumber() const;
};

#endif  // LAMARZOCCO_CLOUD_CLIENT_H