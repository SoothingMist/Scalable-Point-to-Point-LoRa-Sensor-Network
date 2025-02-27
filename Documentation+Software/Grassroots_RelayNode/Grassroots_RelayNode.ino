
// Grassroots Relay Node.
// Polls for available messages.
// Rebroadcasts messages as appropriate

// For viewing Serial.println text on serial monitor.
#define DEBUG

// Unique address of this network node.
// Relays always have adderess of 0
// since they are not sources nor destinations.
#define localAddress 00

// Establish message-tracking table.
// Allows for ignoring older messages.
// Process assumes low message rate.
// This is normal per LoRa basis.
// Network size depends on device memory size.
// Node addresses are 1 .. MAX_NUM_NODES.
// As written, system accommodates 256 unique
// node addresses, 0 .. 255.
#define MAX_NUM_NODES 24
int MessageTrackingTable[MAX_NUM_NODES + 1];

// Library for message handling.
// Initializes LoRa library.
#include "MessageHandler.h"
MessageHandler *MessagingLibrary = NULL;

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) Wait(5000); // wait for serial port to be ready
  #ifdef DEBUG
    Serial.println("Node is active");
  #endif
  
  // Initialize Messaging and LoRa libraries
  MessagingLibrary = new MessageHandler(localAddress);

  // Initialize message tracking table
  for(int i = 0; i <= MAX_NUM_NODES; i++)
    MessageTrackingTable[i] = 0;

  // Ready
  #ifdef DEBUG
    Serial.println("======================================================================");
    Serial.println("Arduino MKR 1310 Grassroots LoRa flood-messaging relay node test");
    Serial.println("======================================================================\n");
  #endif
}

void loop()
{
  // Check for incoming messages.
  // Rebroadcast messages as appropriate.
  if(MessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();

    // Ignore messages with a source address outside the
    // message tracking table.
    if(thisMessage[LOCATION_SOURCE_ID] > MAX_NUM_NODES)
    {
      #ifdef DEBUG
        Serial.println("Source ID exceeds node address limit");
      #endif
      return;
    }

    // Ignore messages whose rebroadcast counter has expired.
    if(thisMessage[LOCATION_REBROADCASTS] == 00)
    {
      #ifdef DEBUG
        Serial.println("Rebroadcast counter is zero");
      #endif
      return;
    }
      
    // Get incremental message ID
    uint16_t thisMessageID = thisMessage[LOCATION_MESSAGE_ID];
    thisMessageID = (thisMessageID << 8) | thisMessage[LOCATION_MESSAGE_ID + 1];

    // Ignore older messages.
    // Accept messages with the current message ID.
    if(thisMessageID < MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]])
    {
      #ifdef DEBUG
        Serial.println("Old message");
      #endif
      return;
    }
    else MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]] = thisMessageID;
    
    // Rebroadcast messages that pass muster.
    #ifdef DEBUG
      Serial.println("Rebroadcasting");
    #endif
    MessagingLibrary->RelayMessage();
    #ifdef DEBUG
      Serial.println();
    #endif
  }
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void Wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
