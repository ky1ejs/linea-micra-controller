#ifndef SH1107_DRIVER_H
#define SH1107_DRIVER_H

#include <Adafruit_SH110X.h>
#include <Wire.h>

#include "../DisplayInterface.h"

class SH1107Driver : public DisplayInterface {
private:
  Adafruit_SH1107 display_;
  uint8_t i2c_addr_;

public:
  SH1107Driver(uint8_t i2c_addr = 0x3C);
  ~SH1107Driver() override = default;

  // Display lifecycle
  bool init() override;
  void clear() override;
  void display() override;

  // Display properties
  uint16_t getWidth() const override;
  uint16_t getHeight() const override;

  // Text operations
  void setCursor(int16_t x, int16_t y) override;
  void setTextSize(float size) override;
  void setTextColor(DisplayColor color) override;
  void setTextWrap(bool wrap) override;
  void print(const String& text) override;
  void println(const String& text) override;

  // Drawing operations
  void setPixel(int16_t x, int16_t y, DisplayColor color) override;
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) override;

  // Display configuration
  void setRotation(uint8_t rotation) override;

private:
  uint16_t convertColor(DisplayColor color) const;
};

#endif