
#include "MessageHandling.h" // class declaration

// Constructor
MessageHandling::MessageHandling()
{
  // Initialize the message-tracking vector.
  for(uint8_t i = 0; i < MAX_NODES; i++) LatestMessageID[i] = 0;
}

// Pass messages through as appropriate.
bool MessageHandling::CheckForRebroadcast()
{
  if(MESSAGE[LOCATION_SYSTEM_ID] != SYSTEM_ID)
  {
    #ifdef DEBUG
      Serial.printf("%s (%3d / %3d)\n", "Message not for this system.",
       (int)MESSAGE[LOCATION_SYSTEM_ID], (int)SYSTEM_ID);
    #endif
    return false;
  }

  if(MESSAGE[LOCATION_SOURCE_ID] < MAX_NODES) // can only handle a maximum number of network nodes
  {
    if(MESSAGE[LOCATION_REBROADCASTS] > 00) // if time-to-live exceeded, do not rebroadcast
    {
      #ifdef DEBUG
        Serial.print("\tCurrent/Previous Message ID: ");
        Serial.print(GetMessageID()); Serial.print(" / ");
        Serial.println(LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]]);
      #endif

      if(GetMessageID() > LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]]) // do not rebroadcast earlier messages
      {
        // Update message table
        if (MESSAGE[LOCATION_MESSAGE_TYPE] == 9) LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]] = 0; //Reset message ID for this node
        else LatestMessageID[MESSAGE[LOCATION_SOURCE_ID]] = GetMessageID();
        
        // Decrement the rebroadcasst counter
        MESSAGE[LOCATION_REBROADCASTS] -= 1;
              
        // Rebroadcast the message
	      return true;
      }
      else return false;
    }
    else return false;
  }
  else return false;
}

// Retrieve the message's ID
uint16_t MessageHandling::GetMessageID()
{
  return (((uint16_t)MESSAGE[LOCATION_MESSAGE_ID]) << 8) |
         (uint16_t)MESSAGE[LOCATION_MESSAGE_ID + 1];
}
