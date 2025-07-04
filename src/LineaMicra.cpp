

#include "LineaMicra.h"

LineaMicra::LineaMicra(HomeAssistantClient *client)
    : haClient(client), isOn(false), temperature(0.0f),
      preBrewIsOn(false), preBrewTime(0.0f), preBrewWait(0.0f) {};

void LineaMicra::deinit()
{
  // Perform any necessary cleanup here
  haClient = nullptr;
  isOn = false;
  temperature = 0.0f;
  preBrewIsOn = false;
  preBrewTime = 0.0f;
  preBrewWait = 0.0f;
}