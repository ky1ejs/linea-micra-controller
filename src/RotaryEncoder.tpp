#include "RotaryEncoder.h"

// You MUST define the static member for each template instantiation
template <int ID>
RotaryEncoder<ID> *RotaryEncoder<ID>::instance = nullptr;

template <int ID>
RotaryEncoder<ID>::RotaryEncoder(int pin_a, int pin_b, int pin_sw, int counts_per_detent)
    : pin_a(pin_a), pin_b(pin_b), pin_sw(pin_sw), counts_per_detent(counts_per_detent), _value(0), value(0),
      prev_a_state(0), prev_b_state(0)
{
  instance = this;
}

template <int ID>
bool RotaryEncoder<ID>::begin()
{
  pinMode(pin_a, INPUT_PULLUP);
  pinMode(pin_b, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin_a), handleEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_b), handleEncoderISR, CHANGE);

  Serial.printf("RotaryEncoder %d initialized on pins A=%d, B=%d\n", ID, pin_a, pin_b);

  return true;
}

template <int ID>
void RotaryEncoder<ID>::handleEncoderISR()
{
  static volatile uint8_t last_encoded = 0;

  uint8_t encoded = (digitalRead(instance->pin_a) << 1) | digitalRead(instance->pin_b);
  uint8_t sum = (last_encoded << 2) | encoded;

  // Only increment/decrement on valid transitions
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  {
    instance->_value++;
  }
  else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  {
    instance->_value--;
  }

  last_encoded = encoded;

  long new_value;
  int counts_per_detent = instance->counts_per_detent;

  // Use proper rounding for division to handle zero-crossing
  if (instance->_value >= 0)
  {
    new_value = (instance->_value + counts_per_detent / 2) / counts_per_detent;
  }
  else
  {
    new_value = (instance->_value - counts_per_detent / 2) / counts_per_detent;
  }

  instance->value = new_value;
}

template <int ID>
long RotaryEncoder<ID>::getValue()
{
  noInterrupts();
  long current_value = value;
  interrupts();

  return current_value;
}
