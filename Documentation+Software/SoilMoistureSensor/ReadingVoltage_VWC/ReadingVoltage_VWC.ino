
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
}

void loop()
{
  // Read the input pin
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;

  // Convert voltage to Volumetric Water Content (VWC)
  float VWC = GetVWC(volts);

  // Plot the data
  // https://forum.arduino.cc/t/fixing-the-y-axis-on-the-serial-plotter-can-it-be-done-yes-it-can-sort-of/431095/2
  // https://www.youtube.com/watch?v=4AQg4vZ_vZI
  // https://github.com/vastevenson/multi-line-plots-arduino-demo
  Serial.print("Sensor_Volts=" + String(volts));
  Serial.println(",VWC=" + String(VWC));
  if(volts != (float)NULL)
    Serial.print(volts, 1); Serial.print(","); Serial.println(VWC, 1);

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 1000);
}

// Takes voltage as input, locates it in the table, applies the equation of a line, and delivers VWC.
float GetVWC(float volts)
{
  const long numRows = 13;
  float table[numRows][2] =
  { // (x_volts, y_VWC)
    {0.0f,  0.0f},
    {0.1f,  0.1f},
    {0.6f,  5.0f},
    {1.1f, 10.0f},
    {1.3f, 15.0f},
    {1.4f, 20.0f},
    {1.5f, 25.0f},
    {1.6f, 30.0f},
    {1.7f, 35.0f},
    {1.8f, 40.0f},
    {2.0f, 45.0f},
    {2.3f, 50.0f},
    {3.0f, 60.0f}
  };

  long r;
  for (r = 1; r < numRows; r++)
  {
    if ((volts >= table[r - 1][0]) &&
      (volts <= table[r][0]))
      break;
  }

  if (r == numRows) return (float)NULL;
  else
  {
    float slope =
      (table[r][1] - table[r - 1][1]) / (table[r][0] - table[r - 1][0]);

    float VWC = slope * (volts - table[r - 1][0]) + table[r - 1][1];

    return VWC;
  }
}
