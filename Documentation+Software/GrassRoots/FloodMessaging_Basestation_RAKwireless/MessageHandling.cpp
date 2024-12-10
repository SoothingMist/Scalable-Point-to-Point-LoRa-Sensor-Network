
#include "MessageHandling.h" // class declaration

// Constructor
MessageHandling::MessageHandling()
{
  // Initialize the message-tracking vector.
  for(uint8_t i = 0; i < MAX_NODES; i++) LatestMessageID[i] = 0;
}

// Pass messages through as appropriate.
bool MessageHandling::CheckForPassthrough()
{
  if(MESSAGE[LOCATION_SYSTEM_ID] != SYSTEM_ID) // only accept messages from a specified system
  {
    #ifdef DEBUG
      Serial.printf("%s (%3d / %3d)\n", "Message not for this system.",
       (int)MESSAGE[LOCATION_SYSTEM_ID], (int)SYSTEM_ID);
    #endif
    return false;
  }
  else if(MESSAGE[LOCATION_DESTINATION_ID] != NODE_ID)
  {
    #ifdef DEBUG
      Serial.printf("%s (%3d / %3d)\n", "Message not for this node.",
       (int)MESSAGE[LOCATION_DESTINATION_ID], (int)NODE_ID);
    #endif
    return false;
  }
  else if(MESSAGE[LOCATION_SOURCE_ID] >= MAX_NODES) // can only handle a maximum number of network nodes
  {
    return false;
  }
  else if(GetMessageID() <= LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]]) // reject earlier messages
  {
    return false;
  }
  else // pass this message forward
  {
    // Update message table
    LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]] = GetMessageID();

    return true;
  }
}

// Retrieve the message's ID
uint16_t MessageHandling::GetMessageID()
{
  return (((uint16_t)MESSAGE[LOCATION_MESSAGE_ID]) << 8) |
         (uint16_t)MESSAGE[LOCATION_MESSAGE_ID + 1];
}
