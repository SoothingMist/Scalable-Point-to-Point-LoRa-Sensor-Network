/*
  LoRa Send/Receive Example

  Sends a message at random intervals.
  Polls continually for incoming messages.
*/

// For viewing Serial.println text on serial monitor.
#define DEBUG

// Get the local and destination addresses for this node.
#include "Addresses.h"

// Library for message handling.
// Includes LoRa library.
#include <LoRaMessageHandler.h>
LoRaMessageHandler *MessagingLibrary = NULL;

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
  while (!Serial) delay(100); // wait for serial port to be ready
  #ifdef DEBUG
    Serial.println("Node is active");
  #endif
  
  // Initialize message-handling library.
  MessagingLibrary = new LoRaMessageHandler(localAddress);
  #ifdef DEBUG
    Serial.println("Transceiver is active (Node " + String(localAddress) + ")");
  #endif
  
  // ================ End Standard Setup ===========================

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();
  
  // Ready
  #ifdef DEBUG
    Serial.println("===============================================");
    Serial.println("Arduino MKR WAN 1310 LoRa Send/Receive test");
    Serial.println("===============================================");
  #endif
}

void loop()
{
  static uint8_t counter = 0;
  
  // Check if it is time to send another message
  if (millis() - lastSendTime > interval)
  {
    // Compose message
    String message = "Hello LoRa World! (" + String(counter++) + ")";
    #ifdef DEBUG
      Serial.println("\nSending: " + message);
    #endif

    // Send text message
    MessagingLibrary->SendTextMessage(message, destinationAddress);
    #ifdef DEBUG
      const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();
      Serial.print("Sent: ");
      for(uint8_t c = 0;
          c < thisMessage[LOCATION_MESSAGE_LENGTH] - MESSAGE_HEADER_LENGTH;
          c++)
        Serial.print((char)thisMessage[MESSAGE_HEADER_LENGTH + c]);
      Serial.println();
    #endif

    // Select the next time to send a message
    lastSendTime = millis();
    interval = random(maxInterval);
  }

  // Check for an incoming packet for this node
  if(MessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();
    #ifdef DEBUG
      Serial.print("\nReceived: '");
      for(uint8_t c = 0;
          c < thisMessage[LOCATION_MESSAGE_LENGTH] - MESSAGE_HEADER_LENGTH;
          c++)
        Serial.print((char)thisMessage[MESSAGE_HEADER_LENGTH + c]);
      Serial.println("' from Node " + String(thisMessage[LOCATION_SOURCE_ID]));
    #endif
  }
}

// ============= Function Definitions =================

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
