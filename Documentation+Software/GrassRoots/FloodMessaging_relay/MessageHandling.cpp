
#include "MessageHandling.h" // class declaration

// Constructor
MessageHandling::MessageHandling()
{
  // Initialize the message-tracking vector.
  for(uint8_t i = 0; i < MAX_NODES; i++) LatestMessageID[i] = 0;
}

// Pass messages through as appropriate.
bool MessageHandling::CheckForRebroadcast(uint16_t size)
{
  // Messages to be rejected

  if(size <= MESSAGE_HEADER_LENGTH) 
  {
    #ifdef DEBUG
      Serial.printf("Message too short for this system.\n");
    #endif
    return false;
  }

  if(MESSAGE[LOCATION_SYSTEM_ID] != SYSTEM_ID) 
  {
    #ifdef DEBUG
      Serial.printf("Message for a different system.\n");
    #endif
    return false;
  }

  if(MESSAGE[LOCATION_SOURCE_ID] > MAX_NODES) 
  {
    #ifdef DEBUG
      Serial.printf("Source ID outside system capacity.");
    #endif
    return false;
  }

  if(MESSAGE[LOCATION_REBROADCASTS] <= 00) // if time-to-live exceeded, do not rebroadcast
  {
    #ifdef DEBUG
      Serial.printf("No further rebroadcasts allowed.\n");
    #endif
    return false;
  }

  if(GetMessageID() <= LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]]) // do not rebroadcast earlier messages
  {
    #ifdef DEBUG
      Serial.printf("Not allowed to rebroadcast current or historical messages.\n");
    #endif
    return false;
  } 
 
  // Message accepted for forwarding

  #ifdef DEBUG
    Serial.print("Message accepted for forwarding (Current/Previous Message ID): ");
    Serial.print(GetMessageID()); Serial.print(" / ");
    Serial.println(LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]]);
  #endif

  // Update message table
  if (MESSAGE[LOCATION_MESSAGE_TYPE] == 9) LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]] = 0; //Reset message ID for this node
  else LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]] = GetMessageID();
  
  // Decrement the rebroadcasst counter
  MESSAGE[LOCATION_REBROADCASTS] -= 1;
          
  return true;
}

// Retrieve the message's ID
uint16_t MessageHandling::GetMessageID()
{
  return (((uint16_t)MESSAGE[LOCATION_MESSAGE_ID]) << 8) |
         (uint16_t)MESSAGE[LOCATION_MESSAGE_ID + 1];
}
