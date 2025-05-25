/*
  LoRa Duplex communication

  Sends a message at random intervals.
  Polls continually for new incoming messages.
  Implements a one-byte addressing scheme.

  Based on original LoRaDuplex example
  by Tom Igoe in baseline Sandeep library.
  https://github.com/sandeepmistry/arduino-LoRa
*/

// Required for LoRa.
// Includes Arduino.h
// A modification of Sandeep's baseline library.
// https://github.com/sandeepmistry/arduino-LoRa.
// His baseline will not work in this program.
// Use the version provided.
// See documentation for details.
#include <LoRa.h>

// Establishes source and destination node addresses.
// The code is otherwise the same for all nodes.
#include "Addresses.h"

String outgoing;                // outgoing message
byte msgCount = 0;              // count of outgoing messages
long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // interval between sends

void setup()
{
  // ============ Standard MKR WAN 1310 Setup Code =============
  
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) wait(5000); // wait for serial port to be ready
  Serial.println("Node is active");
  
  // Initialize LoRa transceiver.
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(915E6))
  {
    Serial.println("Starting LoRa failed!");
    wait(1000);
    exit(1);
  }
  Serial.println("Transceiver is active");

  // From calculator using payload size of 222 and overhead of 13.
  // https://avbentem.github.io/airtime-calculator/ttn/us915/222
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);

  // ================ End Standard Setup ===========================

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();
  
  // Ready
  Serial.println("===========================================");
  Serial.println("Arduino MKR WAN 1310 LoRa Send/Receive test");
  Serial.println("===========================================");
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    String message = "Hello LoRa World!";   // send a message
    sendMessage(message);
    lastSendTime = millis();                // select the next time to send
    interval = random(maxInterval);
  }

  // look for an incoming packet, parse the result:
  onReceive(LoRa.parsePacket());
}

// ============= Function Definitions =================

// Send a packet.
void sendMessage(String outgoing)
{
  while(LoRa.rxSignalDetected()) wait(100); // wait for clear channel
  LoRa.beginPacket();                       // start packet
  LoRa.write(destination);                  // add destination address
  LoRa.write(localAddress);                 // add sender address
  LoRa.write(msgCount);                     // add message ID
  LoRa.write(outgoing.length());            // add payload length
  LoRa.print(outgoing);                     // add payload
  LoRa.endPacket();                         // finish packet and send it
  Serial.println("Sent: " + outgoing + "\n");
  msgCount++;                               // increment message ID
}

// Look for an incoming packet. Parse if present.
void onReceive(int packetSize)
{
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available())
    incoming += (char)LoRa.read();

  if (incomingLength != incoming.length()) // check length for error
  {
    Serial.println("error: message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress)
  {
    Serial.println("This message is not for me.");
    return; // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
