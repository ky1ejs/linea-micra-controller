#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#if defined(BOARD_SDA) && defined(BOARD_SCL)

#include <Wire.h>

#define WIRE Wire

void initI2C();
int scanI2CDevices();

#endif

#endif