

#include "LineaMicra.h"
#include "config.h"

LineaMicra::LineaMicra(HomeAssistantClient *client)
    : haClient(client), _isOn(false), _boilerTemperature(0.0f),
      _preBrewIsOn(false), _preBrewTime(0.0f), _preBrewWait(0.0f)
{
    client->subscribeToEvent(
        LINEA_MICRA_ENTITY_ID,
        [this](const HAEntity &entity)
        {
            // Handle the state change for Linea Micra
            if (entity.state == "on")
            {
                _isOn = true;
            }
            else if (entity.state == "off")
            {
                _isOn = false;
            }
            Serial.println("Linea Micra status updated: " + entity.state);
        });

    client->subscribeToEvent(
        LINEA_MICRA_BOILER_TEMP_ENTITY_ID,
        [this](const HAEntity &entity)
        {
            float newTemperature = entity.state.toFloat();
            // round to 0.1 degree precision
            newTemperature = round(newTemperature * 10.0f) / 10.0f;
            _boilerTemperature = newTemperature;
            Serial.println("Linea Micra target temperature updated: " + String(newTemperature));
        });

    client->subscribeToEvent(
        LINEA_MICRA_PREBREW_TIME_ENTITY_ID,
        [this](const HAEntity &entity)
        {
            _preBrewTime = entity.state.toFloat();
            Serial.println("Linea Micra pre-brew time updated: " + String(_preBrewTime));
        });

    client->subscribeToEvent(
        LINEA_MICRA_PREBREW_WAIT_ENTITY_ID,
        [this](const HAEntity &entity)
        {
            _preBrewWait = entity.state.toFloat();
            Serial.println("Linea Micra pre-brew wait time updated: " + String(_preBrewWait));
        });

    client->subscribeToEvent(
        LINEA_MICRA_PREBREW_MODE_ENTITY_ID,
        [this](const HAEntity &entity)
        {
            Serial.println("Linea Micra pre-brew infusion mode updated: " + entity.state);
            if (entity.state == "prebrew")
            {
                _preBrewIsOn = true;
            }
            else if (entity.state == "disabled")
            {
                _preBrewIsOn = false;
            }
        });
}

LineaMicra::~LineaMicra()
{
    // Cleanup if necessary
    // Currently, no dynamic memory allocation is used, so nothing to free
}

bool LineaMicra::isOn()
{
    return _isOn;
}

float LineaMicra::getBoilerTemperature()
{
    return _boilerTemperature;
}

float LineaMicra::getPreBrewTime()
{
    return _preBrewTime;
}

float LineaMicra::getPreBrewWait()
{
    return _preBrewWait;
}
bool LineaMicra::isPreBrewOn()
{
    return _preBrewIsOn;
}
