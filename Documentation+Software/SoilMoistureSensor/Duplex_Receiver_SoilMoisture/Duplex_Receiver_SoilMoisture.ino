
/*
  LoRa Duplex communication

  Polls continually for new incoming messages.
  Parses and displays sensor data.

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

  // Ready
  Serial.println(F("==============================================="));
  Serial.println(F("Arduino MKR 1310 LoRa sensor data receive parse"));
  Serial.println(F("==============================================="));
}

void loop()
{
  // Look for an incoming packet, parse the result.
  onReceive(LoRa.parsePacket());
}

// ============= Function Definitions =================

// Look for an incoming packet. Parse if present.
void onReceive(int packetSize)
{
  if (packetSize == 0) return;     // if there is no packet, return

  // read packet header bytes:
  uint8_t recipient = LoRa.read(); // recipient address
  
  // Reject if the recipient is not this device
  if (recipient != localAddress)
  {
    Serial.println(F("This message is not for me."));
    while (LoRa.available()) LoRa.read();
    return;
  }

  uint8_t sender = LoRa.read();               // sender address
  uint8_t incomingMsgId = LoRa.read();        // incoming msg ID
  const uint8_t incomingLength = LoRa.read(); // incoming msg length
  unsigned char incomingMessage[incomingLength];

  // Read incoming message
  if(incomingLength > 0)
  {
    uint8_t i = 0;
    while (LoRa.available())
    {
      if (i >= incomingLength) // check length for error
      {
        Serial.println(F("error: payload length does not match stated message length"));
        while (LoRa.available()) LoRa.read();
        return;                // skip rest of function
      }
      incomingMessage[i++] = (unsigned char)LoRa.read();
    }
  }
  else return;
  
  // If message is for this device, print details:
  Serial.println(F("==============="));
  Serial.print(F("Received from: 0x")); Serial.println(String(sender, HEX));
  Serial.print(F("Sent to: 0x")); Serial.println(String(recipient, HEX));
  Serial.print(F("Message ID: ")); Serial.println(String(incomingMsgId));
  Serial.print(F("Message length: ")); Serial.println(String(incomingLength));
  Serial.print(F("Received: ")); Serial.print((char*)incomingMessage); Serial.print(F("\n"));

  // Messages assumed to be in the form  <sensor name> : <sensor value>
  String messageString = (char*)incomingMessage;
  int colon = messageString.indexOf(':');
  String sensorName = messageString.substring(0, colon);
  float sensorValue = (float)atof(messageString.substring(colon + 1, messageString.length()).c_str());
  Serial.print(F("Sensor Name: ")); Serial.print(sensorName); Serial.print(F(", Sensor Value: "));
  Serial.println(sensorValue, 6);
  Serial.print(F("RSSI: ")); Serial.print(String(LoRa.packetRssi()));
  Serial.print(F("  Snr: ")); Serial.println(String(LoRa.packetSnr()));
  Serial.println(F("==============="));
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
