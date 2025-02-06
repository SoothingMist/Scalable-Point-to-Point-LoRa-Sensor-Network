
#include "MessageConstruction.h" // class declaration

// Constructor
MessageConstruction::MessageConstruction()
{
}

// Starts a message with its header
bool MessageConstruction::StartMessage(uint8_t messageType, uint8_t destination)
{
  sourceMessageID++;
  for(uint8_t i = 0; i <= MAX_MESSAGE_INDEX; i++) MESSAGE[i] = (uint8_t)'\0';
  MESSAGE[LOCATION_SYSTEM_ID] = SYSTEM_ID;
  MESSAGE[LOCATION_SOURCE_ID] = SOURCE_NODE_ID;
  MESSAGE[LOCATION_DESTINATION_ID] = destination;
  MESSAGE[LOCATION_MESSAGE_ID] = (uint8_t)(sourceMessageID >> 8); // high byte
  MESSAGE[LOCATION_MESSAGE_ID + 1] = (uint8_t)((sourceMessageID << 8) >> 8); // low byte
  MESSAGE[LOCATION_MESSAGE_TYPE] = messageType;
  MESSAGE[LOCATION_REBROADCASTS] = 05; // number of times to rebroadcast this message
  MessageIndex = MESSAGE_HEADER_LENGTH;
  return true;
}

// Send all segments of the image's pixels
bool MessageConstruction::ComposeMessage_003(uint8_t destination, char* thisText)
{
  StartMessage(003, destination);
  MESSAGE[LOCATION_SENSOR_ID] = 0; // a pure text message
  size_t length = strlen(thisText);
  if(length <= NumFreeBytes)
  {
    memcpy(MESSAGE + MESSAGE_HEADER_LENGTH, (uint8_t*)thisText, length);
    MESSAGE[0] = MESSAGE_HEADER_LENGTH + length - 1; // last cell index in message vector
    return true;
  }
  else
  {
    memcpy(MESSAGE + MESSAGE_HEADER_LENGTH, (uint8_t*)thisText, NumFreeBytes);
    MESSAGE[0] = MESSAGE_HEADER_LENGTH + NumFreeBytes - 1;
    return false;
  }
}
