#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include <Arduino.h>

enum class DisplayColor { BLACK = 0, WHITE = 1 };

enum class TextSize { SMALL = 1, MEDIUM = 2, LARGE = 3 };

class DisplayInterface {
public:
  virtual ~DisplayInterface() = default;

  // Display lifecycle
  virtual bool init() = 0;
  virtual void clear() = 0;
  virtual void display() = 0;

  // Display properties
  virtual uint16_t getWidth() const = 0;
  virtual uint16_t getHeight() const = 0;

  // Text operations
  virtual void setCursor(int16_t x, int16_t y) = 0;
  virtual void setTextSize(float size) = 0;
  virtual void setTextColor(DisplayColor color) = 0;
  virtual void setTextWrap(bool wrap) = 0;
  virtual void print(const String& text) = 0;
  virtual void println(const String& text) = 0;

  // Drawing operations
  virtual void setPixel(int16_t x, int16_t y, DisplayColor color) = 0;
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) = 0;
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) = 0;

  // Display configuration
  virtual void setRotation(uint8_t rotation) = 0;
};

#endif