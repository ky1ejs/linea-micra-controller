#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Wire.h>

#define WIRE Wire

void initI2C();
int scanI2CDevices();

#endif