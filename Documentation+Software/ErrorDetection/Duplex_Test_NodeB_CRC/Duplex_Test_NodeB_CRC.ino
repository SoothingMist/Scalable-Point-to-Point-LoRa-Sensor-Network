
/*
  LoRa Duplex communication

  Sends a message at random intervals.
  Polls continuallyfor new incoming messages.
  Implements a one-byte addressing scheme.

  Based on original LoRaDuplex example
  by Tom Igoe in baseline Sandeep library.
  https://github.com/sandeepmistry/arduino-LoRa

  Adds CRC based on
  https://tanzolab.tanzilli.com/crc
  
*/

// Required for LoRa and Arduino.
// A modification of Sandeep's library.
// His exact library will not work here.
// Use the version provided.
#include "LoRa.h"

// Establishes source and destination node addresses.
// The code is otherwise the same for all nodes.
#include "Addresses.h"

// Add ability to check remaining memory.
// https://docs.arduino.cc/learn/programming/memory-guide
extern "C" char* sbrk(int incr);

// A generated CRC algoritnm.
// Use the one provided.
// Could rewrite for one of your own.
#include "crc-16-dnp.h"

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

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(minInterval, maxInterval);
  lastSendTime = millis();
  
  // Ready
  Serial.println(F("=============================================="));
  Serial.println(F("Arduino MKR 1310 LoRa Send/Receive CRC test"));
  Serial.println(F("=============================================="));
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    // Construct a message
    const unsigned int messageLength = 16;
    char message[messageLength];
    for(unsigned int i = 0; i < messageLength; i++) message[i] = ' ';
    sprintf(message, "%d", freeRam());
    //Serial.print(F("Message Length: ")); Serial.println(messageLength);
    Serial.print(F("Sending: ")); Serial.println(message);

    sendMessage((unsigned char*)message, (uint8_t)messageLength); // LoRa messages are very short

    lastSendTime = millis(); // select the next time to send
    interval = random(minInterval, maxInterval);
  }

  // look for an incoming packet, parse the result:
  onReceive(LoRa.parsePacket());
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
  // Generate CRC value
  uint16_t CRC = crcr16dnp((uint8_t*)outgoing, (int)length, 0);
  
  LoRa.beginPacket();                 // start packet
  LoRa.write(destination);            // add destination address
  LoRa.write(localAddress);           // add sender address
  LoRa.write(msgCount);               // add message ID
  LoRa.write(CRC >> 8);               // CRC high byte
  LoRa.write((CRC << 8) >> 8);        // CRC low byte
  LoRa.write(length);                 // add payload length
  for(uint8_t i = 0; i < length; i++) // add payload
    LoRa.write(outgoing[i]);
  while(CAD()) Wait(500);             // Wait for clear channel
  LoRa.endPacket();                   // finish packet and send it
  msgCount++;                         // increment message ID
  Serial.print(F("Sent:    ")); Serial.print((char*)outgoing);
  Serial.print(F("  (CRC ")); Serial.print(CRC); Serial.println(F(")"));
}

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
  uint16_t CRC = (uint16_t)LoRa.read();       // CRC high byte
  CRC = CRC << 8;
  uint16_t CRC_low =  (uint16_t)LoRa.read();  // CRC low byte
  //Serial.println((String)CRC + "  " + (String)CRC_low);
  CRC = CRC | CRC_low;
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
  Serial.print(F("Received: ")); Serial.print((char*)incomingMessage); Serial.print(F("  "));
  uint16_t CRC_Check = crcr16dnp((uint8_t*)incomingMessage, (int)incomingLength, 0);
  Serial.print(CRC); Serial.print(F("  ")); Serial.println(CRC_Check);
  if(CRC == CRC_Check) Serial.println(F("CRC Passed")); else Serial.println(F("CRC Failed"));
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

// Check remaining memory.
int freeRam()
{
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
