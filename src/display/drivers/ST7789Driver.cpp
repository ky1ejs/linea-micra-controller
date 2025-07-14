#include "ST7789Driver.h"

#include <Arduino.h>

ST7789Driver::ST7789Driver(uint16_t width, uint16_t height)
    : width_(width), height_(height), tft(Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST)), canvas(width_, height_) {}

bool ST7789Driver::init() {
  tft.init(width_, height_);
  Serial.println("ST7789 Display initialized");
  return true;
}

void ST7789Driver::clear() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.setCursor(0, 0);
}

void ST7789Driver::display() {
  // Placeholder - would send buffer to display
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), width_, height_);
}

uint16_t ST7789Driver::getWidth() const {
  return tft.width();
}

uint16_t ST7789Driver::getHeight() const {
  return tft.height();
}

void ST7789Driver::setCursor(int16_t x, int16_t y) {
  canvas.setCursor(x, y);
}

void ST7789Driver::setTextSize(float size) {
  canvas.setTextSize(size);
}

void ST7789Driver::setTextColor(DisplayColor color) {
  canvas.setTextColor(color == DisplayColor::WHITE ? ST77XX_WHITE : ST77XX_BLACK);
}

void ST7789Driver::setTextWrap(bool wrap) {
  canvas.setTextWrap(wrap);
}

void ST7789Driver::print(const String& text) {
  // Placeholder - would render text at current cursor position
  canvas.print(text);
}

void ST7789Driver::println(const String& text) {
  // Placeholder - would render text and move to next line
  canvas.println(text);
}

void ST7789Driver::setPixel(int16_t x, int16_t y, DisplayColor color) {
  // Placeholder - would set pixel color
}

void ST7789Driver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  // Placeholder - would draw rectangle outline
  canvas.drawRect(x, y, w, h, color == DisplayColor::WHITE ? ST77XX_WHITE : ST77XX_BLACK);
}

void ST7789Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  // Placeholder - would draw filled rectangle
  canvas.fillRect(x, y, w, h, color == DisplayColor::WHITE ? ST77XX_WHITE : ST77XX_BLACK);
}

void ST7789Driver::setRotation(uint8_t rotation) {
  // Placeholder - would set display rotation
}