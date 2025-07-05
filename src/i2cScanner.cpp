#include "i2cScanner.h"

#include <Arduino.h>
#include <Wire.h>

void initI2C() {
  WIRE.setPins(BOARD_SDA, BOARD_SCL);
  WIRE.begin();
}

// Inspired by https://www.reddit.com/r/esp32/comments/1ca6y27/comment/l0u8x22/
int scanI2CDevices() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  return nDevices;
}