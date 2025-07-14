#ifndef ST7789_DRIVER_H
#define ST7789_DRIVER_H

#include <Adafruit_ST7789.h>

#include "../DisplayInterface.h"

class ST7789Driver : public DisplayInterface {
private:
  Adafruit_ST7789 tft;
  GFXcanvas16 canvas;
  uint16_t width_;
  uint16_t height_;

public:
  ST7789Driver(uint16_t width = 240, uint16_t height = 320);
  ~ST7789Driver() override = default;

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
};

#endif