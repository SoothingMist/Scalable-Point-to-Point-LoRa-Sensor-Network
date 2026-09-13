
// LoRa Sensor Node. Periodically sends soil-moisture VWC and battery voltage.
// Designed for Arduino MKR WAN 1310 microcontroller with LoRa transceiver.
// For the Arduino MKR WAN 1310, the voltage
// to an input pin cannot exceed 3.3vdc.

// Unique address of this network node.
#define localAddress 1

// Library for LoRa message handling.
#include <LoRaMessageHandler.h>
LoRaMessageHandler *MessagingLibrary = NULL;

// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// The voltage reference is 3.3v.
// https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
#define voltageReference 3.3

// Identify the battery-voltage input pin.
#define batteryPin A2

// Identify the soil=moisture sensor input pin.
#define soilPin A1

// Timing variables.
long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // present interval between sends

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }
  Serial.println("Microprocessor is active");

  // Configure analog/digital conversion (ADC).
  // MKR WAN 1310 is a SAMD board.
  // Default reference is given by AR_DEFAULT.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pins.
  pinMode(batteryPin, INPUT);
  pinMode(soilPin, INPUT);

  // Define and configure LoRa messaging library
  MessagingLibrary = new LoRaMessageHandler(localAddress);

  // Ready
  Serial.println("=====================================================");
  Serial.println("Arduino MKR 1310 LoRa transceiver for battery and VWC");
  Serial.println("=====================================================");
}

void loop()
{
  // Counts the number of packets sent.
  static uint16_t counter = 0;

  // Send sensor values on appropriate schedule.
  if (millis() - lastSendTime > interval)
  {
    // Read the soil-moisture pin.
    // Convert voltage to Volumetric Water Content (VWC).
    float adcValue = (float)analogRead(soilPin);
    float volts = (adcValue / analogResolution) * voltageReference;
    float VWC = GetVWC(volts);
  
    // Read the battery-voltage pin.
    adcValue = (float)analogRead(batteryPin);
    volts = (adcValue / analogResolution) * voltageReference;
    volts *= 2.0f; // see documentation for voltage divider
  
    // Compose message. Broadcast packet.
    char message[100];
    sprintf(message, "DATA: BattV:%5.1f: VWC:%5.1f", volts, VWC);
    String sendString = message;
    Serial.println();
    MessagingLibrary->SendTextMessage(sendString, 3);
    Serial.print("Sent message ");
    Serial.print(++counter);
    Serial.println(" '" + sendString + "'");
    
    // Select the next time to send sensor values.
    lastSendTime = millis();
    interval = random(maxInterval);
  }
}

// Implements piecewise linear equation.
// Takes voltage as input, locates it in the table,
// applies the equation of a line, and delivers VWC.
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
