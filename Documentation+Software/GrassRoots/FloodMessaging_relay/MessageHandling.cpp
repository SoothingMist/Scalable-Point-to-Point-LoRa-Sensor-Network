
#include "MessageHandling.h" // class declaration

// Constructor
MessageHandling::MessageHandling()
{
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

  if(size > 256) 
  {
    #ifdef DEBUG
      Serial.printf("Message too long for this system.\n");
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

  if(MESSAGE[LOCATION_REBROADCASTS] == 00)
  {
    #ifdef DEBUG
      Serial.printf("No further rebroadcasts allowed.\n");
    #endif
    return false;
  }
 
  // Message accepted for rebroadcasting

  #ifdef DEBUG
    Serial.print("Message accepted for rebroadcasting. Current Message ID: ");
    Serial.print(GetMessageID()); Serial.println(".");
  #endif

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
