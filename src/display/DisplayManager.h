#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "DisplayInterface.h"

class DisplayManager {
private:
  DisplayInterface* display_;

public:
  DisplayManager(uint16_t width, uint16_t height);
  ~DisplayManager();

  // Convenience methods that delegate to the active display
  bool init();
  void clear();
  void display();
  uint16_t getWidth() const;
  uint16_t getHeight() const;
  void setCursor(int16_t x, int16_t y);
  void setTextSize(float size);
  void setTextColor(DisplayColor color);
  void setTextWrap(bool wrap);
  void print(const String& text);
  void println(const String& text);
  void setPixel(int16_t x, int16_t y, DisplayColor color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color);
  void setRotation(uint8_t rotation);
};

#endif