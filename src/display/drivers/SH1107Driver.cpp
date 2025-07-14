#include "SH1107Driver.h"

SH1107Driver::SH1107Driver(uint8_t i2c_addr) : display_(64, 128, &Wire), i2c_addr_(i2c_addr) {}

bool SH1107Driver::init() {
  return display_.begin(i2c_addr_);
}

void SH1107Driver::clear() {
  display_.clearDisplay();
}

void SH1107Driver::display() {
  display_.display();
}

uint16_t SH1107Driver::getWidth() const {
  return display_.width();
}

uint16_t SH1107Driver::getHeight() const {
  return display_.height();
}

void SH1107Driver::setCursor(int16_t x, int16_t y) {
  display_.setCursor(x, y);
}

void SH1107Driver::setTextSize(float size) {
  display_.setTextSize(size);
}

void SH1107Driver::setTextColor(DisplayColor color) {
  display_.setTextColor(convertColor(color));
}

void SH1107Driver::setTextWrap(bool wrap) {
  display_.setTextWrap(wrap);
}

void SH1107Driver::print(const String& text) {
  display_.print(text);
}

void SH1107Driver::println(const String& text) {
  display_.println(text);
}

void SH1107Driver::setPixel(int16_t x, int16_t y, DisplayColor color) {
  display_.drawPixel(x, y, convertColor(color));
}

void SH1107Driver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  display_.drawRect(x, y, w, h, convertColor(color));
}

void SH1107Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  display_.fillRect(x, y, w, h, convertColor(color));
}

void SH1107Driver::setRotation(uint8_t rotation) {
  display_.setRotation(rotation);
}

uint16_t SH1107Driver::convertColor(DisplayColor color) const {
  switch (color) {
    case DisplayColor::WHITE:
      return SH110X_WHITE;
    case DisplayColor::BLACK:
    default:
      return SH110X_BLACK;
  }
}