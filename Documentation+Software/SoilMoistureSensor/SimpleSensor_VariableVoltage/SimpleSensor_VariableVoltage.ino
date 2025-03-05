
// This particular sensor node reads variable voltages at the A1 pin.
// Be sure to limit the voltage range to 0 .. 5vdc.
// Output can be viewed on the serial monitor or the serial plotter.

// Adds Arduino's language capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// See setup() for references.
// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f
#define voltageReference 5.0f

// Identify the input pin.
// Positive terminal connected here.
// Negative terminal connected to GND.
#define inputPin A1

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }

  // Configure analog digital conversion (ADC).
  // MKR WAN 1310 is a SAMD board.
  // Default reference is 5v.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pin.
  pinMode(inputPin, INPUT);
}

void loop()
{
  // Read the input pin
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;
  Serial.println(volts);

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 1000);
}
