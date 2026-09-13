
// Basestation Node.
// Polls for available messages.
// Forwards messages to PC as appropriate.
// There are similarities with the
// general-purpose relay node.

// For viewing USB dataflow on serial monitor.
// Useful for testing when not running basestation's PC code.
//#define DEBUG

// Unique address of this network node.
#define localAddress 3

// Establish message-tracking table.
// Allows for ignoring older messages.
// Assumes low message rate from any particular node.
// This is normal per underlying LoRa technology.
// Network size depends on device memory size.
// Node addresses are 1 .. MAX_NUM_NODES.
// As written, system accommodates 256 unique
// node addresses, 1 .. 255. If memory allows
// more, variable byte-sizes have to be expanded.
#define MAX_NUM_NODES 24
uint16_t MessageTrackingTable[MAX_NUM_NODES + 1];

// Library for LoRa message handling.
// Initializes LoRa library.
#include <LoRaMessageHandler.h>
LoRaMessageHandler *MessagingLibrary = NULL;

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) delay(500); // wait for serial port to be ready
  #ifdef DEBUG
    Serial.println("Microcontroller is active");
  #endif
  
  // Initialize Messaging and LoRa libraries
  MessagingLibrary = new LoRaMessageHandler(localAddress);

  // Initialize message tracking table
  for(uint8_t i = 0; i <= MAX_NUM_NODES; i++)
    MessageTrackingTable[i] = 0;

  // Ready
  #ifdef DEBUG
    Serial.println("==========================================================");
    Serial.println("Arduino MKR WAN 1310 LoRa flood-messaging basestation node");
    Serial.println("==========================================================\n");
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
    
    // Get message ID
    uint16_t thisMessageID = thisMessage[LOCATION_MESSAGE_ID];
    thisMessageID = (((uint16_t)thisMessageID) << 8) | 
                    thisMessage[LOCATION_MESSAGE_ID + 1];

    #ifdef DEBUG
      Serial.println("Received Message From Node " +
                     String(thisMessage[LOCATION_SOURCE_ID]) +
                     ". Msg ID: " + String(thisMessageID));
    #endif
    
    // Ignore messages not addressed to this node.
    if(thisMessage[LOCATION_DESTINATION_ID] != localAddress)
    {
      #ifdef DEBUG
        Serial.println("*** Message not for this node");
      #endif
      return;
    }

    // Ignore messages with a source address outside the
    // message tracking table.
    if(thisMessage[LOCATION_SOURCE_ID] > MAX_NUM_NODES)
    {
      #ifdef DEBUG
        Serial.println("*** Source ID exceeds node address limit");
      #endif
      return;
    }

    // Reset message tracking table as appropriate.
    // What maybe needs to happen is for a node to send out
    // a reset message before its message ID counter resets itself.
    // In that way, all relays reset their table for that node,
    // whether or not they see a message ID of zero.
    if(thisMessageID == 0) 
      MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]] = 0;
      
    // Ignore messages already seen.
    if(thisMessageID <= MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]])
    {
      #ifdef DEBUG
        Serial.println("*** Old message (MsgID / TableID) (" + String(thisMessageID) + 
          " / " + String(MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]]) + ")");
      #endif
      return;
    }
    else MessageTrackingTable[thisMessage[LOCATION_SOURCE_ID]] = thisMessageID;
    
    // Pass on to the PC all messages that pass muster.
    #ifdef DEBUG
      Serial.println("Passing message ID " + String(thisMessage[LOCATION_SOURCE_ID]) +
                     "-" +String(thisMessageID) + " to PC");
      if(thisMessage[LOCATION_MESSAGE_TYPE] == 3)
      {
        for(uint8_t i = MESSAGE_HEADER_LENGTH;
            i < thisMessage[LOCATION_MESSAGE_LENGTH];
            i++) Serial.print((char)thisMessage[i]);
        Serial.println('\n');
      }
      else Serial.println("< partial reading, length: " +
             String(thisMessage[LOCATION_MESSAGE_LENGTH]) + " >\n");

    #else
      // when not debugging, write the message to the serial port
      for(uint8_t i = 0; i < thisMessage[LOCATION_MESSAGE_LENGTH]; i++)
        Serial.write(thisMessage[i]);
      Serial.flush();
    #endif
  }
}
