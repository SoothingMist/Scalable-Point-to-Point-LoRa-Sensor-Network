
// Reads the voltage being input to a given pin.

// Adds Arduino's language capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f
#define voltageReference 3.3f

float ReadVoltage(uint8_t voltagePin)
{
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
  // We read the voltage being input to that pin.
  pinMode(voltagePin, INPUT);

  // Read the voltage pin
  float adcValue = (float)analogRead(voltagePin);
  float volts = (adcValue / analogResolution) * voltageReference;

  // Return the determined voltage
  return volts;
}
