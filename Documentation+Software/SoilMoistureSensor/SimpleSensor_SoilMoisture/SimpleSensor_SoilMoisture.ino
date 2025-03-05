
// This particular sensor node reads a soil-moisture sensor.
// Voltage is converted to volumetric moisture content (VMC).
// See documentation for details.
// Output can be viewed on the serial monitor or the serial plotter.

// Adds Arduino's language capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// See setup() for references.
// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// Using maximum value produced by the sensor in question.
#define voltageReference 3.0f

// Identify the input pin.
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
  // Good plotting video and code
  // https://www.youtube.com/watch?v=4AQg4vZ_vZI
  // https://github.com/vastevenson/multi-line-plots-arduino-demo
  
  // Read the input pin
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;
  Serial.print("Volts:");
  Serial.print(volts);
/*
  // Convert volts to VMC
  float VMC = -(17.741f * pow(volts, 4.0f)) 
              +(77.926f * pow(volts, 3.0f))
              -(95.824f * pow(volts, 2.0f))
              +(47.161f * volts) - 3.6598;
  Serial.print(",");
  Serial.print("VMC:");
  Serial.print(VMC);
*/
  Serial.println();
  
  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 500);
}
