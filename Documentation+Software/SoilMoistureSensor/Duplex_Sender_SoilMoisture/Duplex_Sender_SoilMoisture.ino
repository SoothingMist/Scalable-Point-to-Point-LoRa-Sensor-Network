
/*
  LoRa Duplex communication

  Sends a sensor-data message at random intervals.

  Based on original LoRaDuplex example
  by Tom Igoe in baseline Sandeep library.
  https://github.com/sandeepmistry/arduino-LoRa

*/

// Required for LoRa and Arduino.
// A modification of Sandeep's library.
// His exact library will not work here.
// Use the version provided.
#include "LoRa.h"

// Establishes source and destination node addresses.
// The code is otherwise the same for all nodes.
#include "Addresses.h"

uint8_t msgCount = 0;     // count of outgoing messages
long lastSendTime = 0;    // last send time
long minInterval = 2000;  // minimum millisecond interval between sends
long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;        // interval between sends

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
  // ============ Standard MKR WAN 1310 Setup Code =============
  
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) Wait(5000); // wait for serial port to be ready
  Serial.println(F("Node is active"));

  // Initialize LoRa transceiver.
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(915E6))
  {
    Serial.println(F("Starting LoRa failed!"));
    Wait(1000);
    exit(1);
  }
  Serial.println(F("Transceiver is active"));

  // From calculator using payload size of 222 and overhead of 13.
  // https://avbentem.github.io/airtime-calculator/ttn/us915/222
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  Serial.print(F("Frequency: ")); Serial.println(LoRa.getFrequency());
  Serial.print(F("Spreading Factor: ")); Serial.println(LoRa.getSpreadingFactor());
  Serial.print(F("Signal Bandwidth: ")); Serial.println(LoRa.getSignalBandwidth());

  // ================ End Standard Setup ===========================

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(minInterval, maxInterval);
  lastSendTime = millis();
  
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
  
  // Ready
  Serial.println(F("=============================================="));
  Serial.println(F("Arduino MKR 1310 LoRa send soil-moisture data"));
  Serial.println(F("=============================================="));
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    // Read soil-moisture voltage via the input pin
    float adcValue = (float)analogRead(inputPin);
    float volts = (adcValue / analogResolution) * voltageReference;
  
    // Construct a message
    const unsigned int messageLength = 32;
    char message[messageLength];
    for(unsigned int i = 0; i < messageLength; i++) message[i] = ' ';
    sprintf(message, "Soil Moisture: %f", volts);
    Serial.print(F("Sending: ")); Serial.println(message);

    sendMessage((unsigned char*)message, (uint8_t)messageLength); // LoRa messages are very short

    lastSendTime = millis(); // select the next time to send
    interval = random(minInterval, maxInterval);
  }
}

// ============= Function Definitions =================

bool CAD()
{
  // CAD not fully implemented by Sandeep library.
  bool signalDetected = LoRa.rxSignalDetected();
  
  if (signalDetected)
  {
    Serial.println(F("Signal detected"));
  }
  else
  {
    Serial.println(F("No signal detected. Could send something."));
  }
  return signalDetected;
}

// Send a packet.
void sendMessage(unsigned char* outgoing, uint8_t length)
{
  LoRa.beginPacket();                 // start packet
  LoRa.write(destination);            // add destination address
  LoRa.write(localAddress);           // add sender address
  LoRa.write(msgCount);               // add message ID
  LoRa.write(length);                 // add payload length
  for(uint8_t i = 0; i < length; i++) // add payload
    LoRa.write(outgoing[i]);
  while(CAD()) Wait(500);             // Wait for clear channel
  LoRa.endPacket();                   // finish packet and send it
  msgCount++;                         // increment message ID
  Serial.print(F("Sent:    ")); Serial.print((char*)outgoing);
  Serial.println("\n");
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void Wait(long milliseconds)
{
  long beginTime = millis();
  uint8_t doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
