
#ifndef LINEA_MICRA_H
#define LINEA_MICRA_H

#include "HomeAssistantClient.h"
#include "LaMarzoccoCloudClient.h"

enum class LineaMicraClientType { HOME_ASSISTANT, CLOUD_API };

class LineaMicra {
public:
  // Constructor for Home Assistant client
  LineaMicra(HomeAssistantClient* client);

  // Constructor for Cloud API client
  LineaMicra(LaMarzoccoCloudClient* client);

  ~LineaMicra();

  // State getters
  bool isOn();
  float getBoilerTemperature();
  float getPreBrewTime();
  float getPreBrewWait();
  bool isPreBrewOn();

  // State setters (only available with Cloud API)
  bool setPower(bool enabled);
  bool setBoilerTemperature(float temperature);
  bool setPreBrewMode(bool enabled);
  bool setPreBrewTimes(float onTime, float waitTime);

  // Utility
  bool refreshFromCloud();  // Update state from cloud API
  LineaMicraClientType getClientType() const;

private:
  HomeAssistantClient* haClient;
  LaMarzoccoCloudClient* cloudClient;
  LineaMicraClientType clientType;

  volatile bool _isOn;
  float _boilerTemperature;
  bool _preBrewIsOn;
  float _preBrewTime;
  float _preBrewWait;

  void setupHomeAssistantSubscriptions();
  void updateStateFromCloud();
};

#endif  // LINEA_MICRA_H