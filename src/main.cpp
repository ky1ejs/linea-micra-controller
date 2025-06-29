#include "esp_log.h"
#include <stdio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH110X display = Adafruit_SH110X();

void app_main()
{
  while (1)
  {
    ESP_LOGI("main", "Hello, ESP-IDF!");
    // delay(1000); // Delay for 1000 milliseconds (1 second)
  }
}