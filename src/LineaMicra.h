
#ifndef LINEA_MICRA_H
#define LINEA_MICRA_H

#include "HomeAssistantClient.h"

class LineaMicra
{
public:
  LineaMicra(HomeAssistantClient *client);
  ~LineaMicra();

  bool isOn();
  float getBoilerTemperature();
  float getPreBrewTime();
  float getPreBrewWait();
  bool isPreBrewOn();

private:
  HomeAssistantClient *haClient;
  volatile bool _isOn;
  float _boilerTemperature;
  bool _preBrewIsOn;
  float _preBrewTime;
  float _preBrewWait;
};

#endif // LINEA_MICRA_H