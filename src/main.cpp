#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "i2cScanner.h"
#include <Adafruit_seesaw.h>

#define ENCODER1_A 0  // Channel A
#define ENCODER1_B 20 // Channel B
volatile long encoder1_count = 0;
volatile bool encoder1_a_last = HIGH;
volatile unsigned long encoder1_last_time = 0;
const unsigned long DEBOUNCE_TIME = 5000; // 50ms debounce
// Interrupt service routines
void IRAM_ATTR encoder1_ISR()
{
  unsigned long current_time = micros();

  // Debounce check
  // if (current_time - encoder1_last_time < DEBOUNCE_TIME)
  // {
  // return;
  // }
  encoder1_last_time = current_time;

  bool a_state = digitalRead(ENCODER1_A);
  bool b_state = digitalRead(ENCODER1_B);

  // Check if A changed from HIGH to LOW (falling edge)
  if (encoder1_a_last == HIGH && a_state == LOW)
  {
    // Check B state to determine direction
    if (b_state == HIGH)
    {
      encoder1_count++; // Clockwise
    }
    else
    {
      encoder1_count--; // Counter-clockwise
    }
  }
  encoder1_a_last = a_state;
}

void configureEncoder()
{
  pinMode(ENCODER1_A, INPUT_PULLUP);
  pinMode(ENCODER1_B, INPUT_PULLUP);
  attachInterrupt(ENCODER1_A, encoder1_ISR, CHANGE);
  attachInterrupt(ENCODER1_B, encoder1_ISR, CHANGE);
  Serial.println("Encoders initialized. Start turning!");
}

Adafruit_seesaw ss;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
int32_t encoder_position;

#define SEESAW_ADDR 0x36
#define SS_SWITCH 24
#define DISPLAY_ADDR 0x3C

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("\nI2C Scanner");

  initI2C();

  configureEncoder();

  if (!ss.begin(SEESAW_ADDR))
  {
    Serial.println("Couldn't find seesaw on default address");
    while (1)
      delay(10);
  }
  Serial.println("seesaw started");

  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  if (version != 4991)
  {
    Serial.print("Wrong firmware loaded? ");
    Serial.println(version);
    while (1)
      delay(10);
  }
  Serial.println("Found Product 4991");

  // use a pin for the built in encoder switch
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);

  // get starting position
  encoder_position = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  delay(10);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();

  // Initialize the display
  if (!display.begin(DISPLAY_ADDR))
  {
    Serial.println("SH110X allocation failed - continuing without display");
  }
  else
  {
    Serial.println("Display initialized successfully");
    display.display();
    delay(2000);
    display.setRotation(1);
    display.clearDisplay();
    display.setTextSize(1.2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.setTextWrap(true);
    display.println("Is this still working?");
    display.display();
  }
}

void loop()
{
  if (!ss.digitalRead(SS_SWITCH))
  {
    Serial.println("Button pressed!");
  }

  // Store current counts (disable interrupts briefly for atomic read)
  noInterrupts();
  long enc1 = encoder1_count;
  interrupts();

  int32_t new_position = ss.getEncoderPosition();
  // did we move arounde?
  if (encoder_position != new_position)
  {
    Serial.println(new_position); // display new position

    display.clearDisplay();  // clear the display
    display.setCursor(0, 0); // reset cursor position
    display.print("Encoder Position: ");
    display.println(new_position); // print the new position
    display.print("Encoder 2: ");
    display.println(enc1); // print the first encoder count
    display.display();     // update the display
  }

  // scanI2CDevices();
  delay(100); // wait 100 milliseconds for next scan
}
