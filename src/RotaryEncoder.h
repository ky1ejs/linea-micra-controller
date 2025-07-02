/*
 * RotaryEncoder.h - ESP32 Rotary Encoder Library with Detent Filtering
 * Supports PEC11 and similar quadrature encoders with push buttons
 */

#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include <Arduino.h>

template <int ID>
class RotaryEncoder
{
public:
  RotaryEncoder(int pin_a, int pin_b, int pin_sw = -1, int counts_per_detent = 4);
  bool begin();
  long getValue();
  static RotaryEncoder<ID> *instance;
  static void handleEncoderISR();

private:
  int pin_a;
  int pin_b;
  int pin_sw;
  int counts_per_detent;
  long _value;
  long value;

  int prev_a_state;
  int prev_b_state;
};

#include "RotaryEncoder.tpp"

#endif // ROTARY_ENCODER_H