#include "DisplayManager.h"

#ifdef DISPLAY_TYPE_SH1107
#include "drivers/SH1107Driver.h"
#endif

#ifdef DISPLAY_TYPE_ST7789
#include "drivers/ST7789Driver.h"
#endif

DisplayManager::DisplayManager(uint16_t width, uint16_t height) : display_(nullptr) {
  // Compile-time selection of display driver
#ifdef DISPLAY_TYPE_SH1107
  display_ = new SH1107Driver();
#elif defined(DISPLAY_TYPE_ST7789)
  display_ = new ST7789Driver(width, height);
#else
#error "No display type defined. Please define DISPLAY_TYPE_SH1107 or DISPLAY_TYPE_ST7789"
#endif
}

DisplayManager::~DisplayManager() {
  delete display_;
  display_ = nullptr;
}

bool DisplayManager::init() {
  return display_ ? display_->init() : false;
}

void DisplayManager::clear() {
  if (display_)
    display_->clear();
}

void DisplayManager::display() {
  if (display_)
    display_->display();
}

uint16_t DisplayManager::getWidth() const {
  return display_ ? display_->getWidth() : 0;
}

uint16_t DisplayManager::getHeight() const {
  return display_ ? display_->getHeight() : 0;
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
  if (display_)
    display_->setCursor(x, y);
}

void DisplayManager::setTextSize(float size) {
  if (display_)
    display_->setTextSize(size);
}

void DisplayManager::setTextColor(DisplayColor color) {
  if (display_)
    display_->setTextColor(color);
}

void DisplayManager::setTextWrap(bool wrap) {
  if (display_)
    display_->setTextWrap(wrap);
}

void DisplayManager::print(const String& text) {
  if (display_)
    display_->print(text);
}

void DisplayManager::println(const String& text) {
  if (display_)
    display_->println(text);
}

void DisplayManager::setPixel(int16_t x, int16_t y, DisplayColor color) {
  if (display_)
    display_->setPixel(x, y, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  if (display_)
    display_->drawRect(x, y, w, h, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, DisplayColor color) {
  if (display_)
    display_->fillRect(x, y, w, h, color);
}

void DisplayManager::setRotation(uint8_t rotation) {
  if (display_)
    display_->setRotation(rotation);
}