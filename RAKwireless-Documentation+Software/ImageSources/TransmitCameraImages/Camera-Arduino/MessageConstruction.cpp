
#include "MessageConstruction.h" // class declaration

// Constructor
MessageConstruction::MessageConstruction()
{
  // Initialize the camera.
  if(pixy2Camera.init() == PIXY_RESULT_OK)
  {
    pixy2Camera.changeProg("video");
    ImageHeight = pixy2Camera.frameHeight;
    ImageWidth = pixy2Camera.frameWidth;
    #ifdef DEBUG
      Serial.println("\nPixy2 initialized.");
      Serial.print("Camera Height (Rows): "); Serial.println(ImageHeight);
      Serial.print("Camera Width (Columns): "); Serial.println(ImageWidth);
    #endif
  }
  #ifdef DEBUG
    else
    {
        Serial.println("\n*** Camera initialization failed\n");
    }
  #endif
}

// Forward a message
bool MessageConstruction::ForwardMessage()
{
  #ifdef DEBUG
    Serial.print("Forwarded message "); Serial.print(GetMessageID()); 
    Serial.print(" of length "); Serial.println(MessageIndex);
  #else
    MESSAGE[LOCATION_MESSAGE_INDEX] = MessageIndex - 1; // location of last byte in message
    Serial.write(MESSAGE, MessageIndex);
    // These are long messages, up to 256 bytes in length.
    // Tried various approaches using flush and availableForWrite.
    // Also tried manually increasing the size of the write buffer.
    // What worked is simply waiting this period ot time for whatever
    // is going on under the covers to complete.
    unsigned long startTime = millis();
    while(millis() - startTime < 500);
  #endif

  return true;
}

// Starts a message with its header
bool MessageConstruction::StartMessage(uint8_t messageType, uint8_t destination)
{
  sourceMessageID++;
  for(uint8_t i = 0; i < MAX_MESSAGE_INDEX; i++) MESSAGE[i] = (uint8_t)'\0';
  MESSAGE[LOCATION_SYSTEM_ID] = SYSTEM_ID;
  MESSAGE[LOCATION_SOURCE_ID] = SOURCE_NODE_ID;
  MESSAGE[LOCATION_DESTINATION_ID] = destination;
  MESSAGE[LOCATION_MESSAGE_ID] = (uint8_t)(sourceMessageID >> 8); // high byte
  MESSAGE[LOCATION_MESSAGE_ID + 1] = (uint8_t)((sourceMessageID << 8) >> 8); // low byte
  MESSAGE[LOCATION_MESSAGE_TYPE] = messageType;
  MESSAGE[LOCATION_SENSOR_ID] = 1; // assuming one camera due to UnoR3 memory limitations
  MESSAGE[LOCATION_REBROADCASTS] = 05; // number of times to rebroadcast this message
  MessageIndex = MESSAGE_HEADER_LENGTH;
  return true;
}

// Send all segments of the image's pixels
bool MessageConstruction::ComposeMessage_000(uint8_t destination)
{
  // Take snapshot.
  // Pixy2.1 cannot take static images.
  // Therefore, physically, the camera has to be held in a staring, continuous-dwell, position.
  // Motion in the image or the camera's motion changes the image as this code tries to send the image.

  // Paints the image row by row.
  // The number of columns is divided by SegmentSize,
  // the number of pixels that fit within a message envelope.
  // The standard envelope contains MESSAGE_HEADER_LENGTH bytes.
  // The envelope for message type 000 contains an extra 6 bytes.
  const uint8_t SegmentSize = (uint8_t)((NumFreeBytes - 6) / IMAGE_DEPTH);

  // For segmenting the image.
  const uint8_t NumFullColumnBlocks =
    (uint8_t)((ImageWidth - (2 * NUMBER_TO_SKIP)) / SegmentSize); // number of pixels in full column segments
  const uint8_t NumRemainingPixels =
    (uint8_t)((ImageWidth - (2 * NUMBER_TO_SKIP)) % SegmentSize); // number of pixels in the remaining column partial segment

  // Number of rows and columns can be greater than 255 so we have to get low and high bytes of each.
  uint8_t row_HighByte;
  uint8_t row_LowByte;

  // Values of image colors.
  uint8_t red, green, blue;

  // Send messages containing pixels on a row-by-row basis.
  // Segments of each row's columns are selected as pixels to add to the message.
  // Each message contains no more than SegmentSize number of pixels.
  for (uint16_t r = NUMBER_TO_SKIP; r < ImageHeight - NUMBER_TO_SKIP; r++)
  {
    row_HighByte = (uint8_t)(r >> 8);
    row_LowByte = (uint8_t)((r << 8) >> 8);
    uint16_t c = NUMBER_TO_SKIP;

    // Go through each SegmentSize block of pixels
    for (uint8_t cb = 0; cb < NumFullColumnBlocks; cb++)
    {
      // Start with the message header
      StartMessage(0, destination);
      
      // Each message contains a block of column pixels for a given row.
      // row and column indicate where a given segment of pixels starts.
      MESSAGE[MessageIndex++] = row_HighByte;
      MESSAGE[MessageIndex++] = row_LowByte;
      MESSAGE[MessageIndex++] = (uint8_t)(c >> 8); // column high byte
      MESSAGE[MessageIndex++] = (uint8_t)((c << 8) >> 8); // column low byte
      MESSAGE[MessageIndex++] = SegmentSize;
      MESSAGE[MessageIndex++] = IMAGE_DEPTH;

      // Append the pixels in this segment.
      for (uint8_t ss = 0; ss < SegmentSize; ss++)
      {
        // Get this pixel's values
        pixy2Camera.video.getRGB(c, r, &red, &green, &blue, SATURATE);
        MESSAGE[MessageIndex++] = red;
        MESSAGE[MessageIndex++] = green;
        MESSAGE[MessageIndex++] = blue;
        
        // Increment the column counter
        c++;
      }

      // Transmit the message
      ForwardMessage();
    }

    // Send the pixels in the last partial segment of columns
    if(NumRemainingPixels > 0)
    {
      // Start with the message header
      StartMessage(0, destination);
      
      // Each message contains a block of column pixels for a given row.
      // row and column indicate where a given segment of pixels starts.
      MESSAGE[MessageIndex++] = row_HighByte;
      MESSAGE[MessageIndex++] = row_LowByte;
      MESSAGE[MessageIndex++] = (uint8_t)(c >> 8); // column high byte
      MESSAGE[MessageIndex++] = (uint8_t)((c << 8) >> 8); // column low byte
      MESSAGE[MessageIndex++] = NumRemainingPixels;
      MESSAGE[MessageIndex++] = IMAGE_DEPTH;

      for (c = c; c < ImageWidth - NUMBER_TO_SKIP; c++)
      {
        // Get this pixel's values
        pixy2Camera.video.getRGB(c, r, &red, &green, &blue, SATURATE);
        MESSAGE[MessageIndex++] = red;
        MESSAGE[MessageIndex++] = green;
        MESSAGE[MessageIndex++] = blue;
      }
    
      // Transmit the message
      ForwardMessage();
    }
  }
  return true;
}

// Retrieve the message's ID
uint16_t MessageConstruction::GetMessageID()
{
  return (((uint16_t)MESSAGE[LOCATION_MESSAGE_ID]) << 8) |
         (uint16_t)MESSAGE[LOCATION_MESSAGE_ID + 1];
}
