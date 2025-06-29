#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "i2cScanner.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("\nI2C Scanner");

  initI2C();

  // Initialize the display
  if (!display.begin(0x3C))
  {
    Serial.println("SH110X allocation failed - continuing without display");
  }
  else
  {
    Serial.println("Display initialized successfully");
    display.display();
    delay(2000);
    display.setRotation(1);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.setTextWrap(true);
    display.println("kylejs loves Parker and Sabrina");
    display.display();
  }
}

void loop()
{
  scanI2CDevices();
  delay(5000); // wait 5 seconds for next scan
}
