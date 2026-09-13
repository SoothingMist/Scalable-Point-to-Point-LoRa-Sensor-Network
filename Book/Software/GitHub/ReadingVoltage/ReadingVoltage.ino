
// This particular sensor node reads the voltage
// at a given input pin.
// For the Arduino MKR WAN 1310, the input voltage
// to that pin cannot exceed 3.3vdc.

// Adds Arduino's language capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// The voltage reference is 3.3v.
// https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
#define voltageReference 3.3

// Identify the input pin
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
  // Default reference is given by AR_DEFAULT.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pin.
  pinMode(inputPin, INPUT);

  // Ready
  // Designed to run with the serial plotter.
  // Annotates for data source for min,max,and two plots.
  Serial.println("Min_Volts,Max_Volts,Measured_Volts,Actual_Volts");
}

void loop()
{
  // Read the input pin
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;

  // Plot the data
  // https://forum.arduino.cc/t/fixing-the-y-axis-on-the-serial-plotter-can-it-be-done-yes-it-can-sort-of/431095/2
  Serial.print(0); Serial.print(","); Serial.print(10); // freeze lower and upper limit
  Serial.print(","); Serial.print(volts, 1); Serial.print(","); Serial.println(2.0f * volts, 1);

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 1000);
}
