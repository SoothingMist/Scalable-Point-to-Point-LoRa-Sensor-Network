
// Grassroots Relay Node.
// Polls for available messages.
// Rebroadcasts messages as appropriate

// For viewing Serial.println text on serial monitor.
#define DEBUG

// Unique address of this network node.
// Relays always have adderess of 0
// since they are not sources nor destinations.
#define localAddress 0

// Establish message-tracking table.
// Allows for ignoring older messages.
// Process assumes low message rate.
// This is normal per underlying LoRa technology.
// Also due to national governing laws.
// Network size depends on device memory size.
// Node addresses are 1 .. MAX_NUM_NODES.
// As written, system accommodates 256 unique
// node addresses, 0 .. 255.
#define MAX_NUM_NODES 24
uint16_t MessageTrackingTable[MAX_NUM_NODES + 1];

// Library for message handling.
// Initializes LoRa library.
#include <LoRaMessageHandler.h>
LoRaMessageHandler *LoRaMessagingLibrary = NULL;

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }
  #ifdef DEBUG
    Serial.println("Microprocessor is active");
  #endif
  
  // Initialize Messaging and LoRa libraries
  LoRaMessagingLibrary = new LoRaMessageHandler(localAddress);

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
  if(LoRaMessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = LoRaMessagingLibrary->getMESSAGE();

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
    if(thisMessage[LOCATION_REBROADCASTS] == 0)
    {
      #ifdef DEBUG
        Serial.println("Rebroadcast counter is zero");
      #endif
      return;
    }
      
    // Get incremental message ID
    uint16_t thisMessageID = thisMessage[LOCATION_MESSAGE_ID];
    thisMessageID = (thisMessageID << 8) | thisMessage[LOCATION_MESSAGE_ID + 1];

    // Reset message tracking table as appropriate
    if(thisMessageID == 0) 
      MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]] = 0;
      
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
      Serial.print("Rebroadcasting... ");
    #endif
    LoRaMessagingLibrary->RelayMessage();
    #ifdef DEBUG
      Serial.println("Success\n");
    #endif
  }
  //else Serial.println("No packet found");
}
