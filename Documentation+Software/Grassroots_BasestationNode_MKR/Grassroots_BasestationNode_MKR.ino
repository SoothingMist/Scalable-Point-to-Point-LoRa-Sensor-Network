
// Grassroots Basestation Node.
// Polls for available messages.
// Forwards messages to PC as appropriate.

// For viewing Serial.println text on serial monitor.
//#define DEBUG

// Unique address of this network node.
#define localAddress 03

// Establish message-tracking variable.
// Allows for ignoring older messages.
// Process assumes low message rate.
// This is normal per LoRa basis.
uint16_t messageTrackingVariable = 00;

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

  // Ready
  #ifdef DEBUG
    Serial.println("======================================================================");
    Serial.println("Arduino MKR 1310 Grassroots LoRa flood-messaging basetation node test");
    Serial.println("======================================================================\n");
  #endif
}

void loop()
{
  // Check for incoming messages.
  // Forward messages as appropriate.
  if(MessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();

    // Get incremental message ID
    uint16_t thisMessageID = thisMessage[LOCATION_MESSAGE_ID];
    thisMessageID = (thisMessageID << 8) | thisMessage[LOCATION_MESSAGE_ID + 1];

    // Reset message tracking table as appropriate
    if(thisMessageID == 0) 
      messageTrackingVariable = 0;
      
    // Ignore older messages or messages already seen.

    // Subtle but important difference between messages that hold a sensor's
    // entire reading and those that do not.
    if(thisMessage[LOCATION_MESSAGE_TYPE] == 3) // messages with entire reading
    {
      if(thisMessageID <= messageTrackingVariable) // do not forward messages already seen
      {
        #ifdef DEBUG
          Serial.println("Old message: " + (String)thisMessageID + "  " + (String)messageTrackingVariable + "\n");
        #endif
        return;
      }
      else messageTrackingVariable = thisMessageID;
    }
    else // messages that do not hold entire reading
    {
      if(thisMessageID < messageTrackingVariable)
      {
        #ifdef DEBUG
          Serial.println("Old message\n");
        #endif
        return;
      }
      else messageTrackingVariable = thisMessageID;
    }
    
    // Forward messages that pass muster.
    #ifdef DEBUG
      Serial.print("Forwarding: ");
      if(thisMessage[LOCATION_MESSAGE_TYPE] == 3)
      {
        for(uint8_t i = MESSAGE_HEADER_LENGTH;
            i < thisMessage[LOCATION_MESSAGE_LENGTH];
            i++) Serial.print((char)thisMessage[i]);
        Serial.println('\n');
      }
      else Serial.println("< message with partial reading >\n");
    #else // when not debugging, write the message to the serial port
      for(uint8_t i = 0; i < thisMessage[LOCATION_MESSAGE_LENGTH]; i++)
        Serial.write(thisMessage[i]);
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
