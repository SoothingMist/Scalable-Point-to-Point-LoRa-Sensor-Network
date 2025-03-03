
/*
  LoRa Duplex communication

  Sends a message at random intervals.
  Polls continuallyfor new incoming messages.
  Implements a one-byte addressing scheme.

  Based on original LoRaDuplex example
  by Tom Igoe in baseline Sandeep library.
  https://github.com/sandeepmistry/arduino-LoRa

  Adds encrypt/decrypt from
  https://github.com/SergeyBel/AES
  
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

// Ref: https://github.com/SergeyBel/AES
// Use the version provided.
#include "AES.h"
AES aes(AESKeyLength::AES_128); // set key length to 128, can be 128, 192 or 256  
unsigned char key[] = // example key
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

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
  Serial.println(F("Arduino MKR 1310 LoRa Send/Receive cypher test"));
  Serial.println(F("=============================================="));
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    // Construct a message
    const unsigned int messageLength = 16;
    unsigned int numMessageBits = messageLength * sizeof(unsigned char);
    char message[messageLength];
    for(unsigned int i = 0; i < messageLength; i++) message[i] = ' ';
    sprintf(message, "%d", freeRam());
    //Serial.print(F("Message Length: ")); Serial.println(messageLength);
    Serial.print(F("Sending (original): ")); Serial.println(message);

    // encrypt
    unsigned char* encryption = aes.EncryptECB((unsigned char*)message, numMessageBits, key);
    
    sendMessage(encryption, (uint8_t)messageLength); // LoRa messages are very short
    delete [] encryption; // have to delete this after no longer needed to prevent memory leak

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
  
  // Decrypt and repeat message
  unsigned char* decryption = aes.DecryptECB(outgoing, length * sizeof(unsigned char), key); // decrypted message
  Serial.print(F("Sent (decrypted):   "));
  for(uint8_t i = 0; i < length; i++) Serial.print((char)decryption[i]);
  Serial.println();
  delete [] decryption; // have to delete this after no longer needed to prevent memory leak
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

  // Display decrypted message
  Serial.print(F("Message (decrypted): "));
  unsigned char* decryption = aes.DecryptECB(incomingMessage, incomingLength * sizeof(unsigned char), key); // decrypted message
  for(uint8_t i = 0; i < incomingLength; i++) Serial.print((char)decryption[i]);
  delete [] decryption; // have to delete this after no longer needed to prevent memory leak

  Serial.print(F("\nRSSI: ")); Serial.print(String(LoRa.packetRssi()));
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
