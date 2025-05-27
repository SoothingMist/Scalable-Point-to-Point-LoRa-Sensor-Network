
// Transceiver side of Programmable USB Hub.
// Accepts messages from the hub and transmits them.

// Library for message handling.
// Includes LoRa library.
#include <MessageHandler.h>
MessageHandler *messagingLibrary = NULL;

// Unique address of this network node
#define localAddress 2

// Constants and variables used by various subroutines
// Constants and variables regarding messages.
const uint8_t HANDSHAKE = (uint8_t)'H';
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
uint8_t SIZE_IO_BUFFER = 0;

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready

  // Initialize message-handling library.
  messagingLibrary = new MessageHandler(localAddress);

  // Wait for connection with external device
  Connect();

  // Make sure the serial input buffer is empty.
  while(Serial.available()) Serial.read();

  // Get size of this device's serial output buffer
  // https://www.arduino.cc/reference/tr/language/functions/communication/serial/availableforwrite
  SIZE_IO_BUFFER = Serial.availableForWrite();

  // Send request for entire image
  MESSAGE[0] = 2;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();
}

void loop()
{
  static bool getAnotherSegment = true;
  if(getAnotherSegment) // stop upon "Done" message
  {
    // Send "ready for next segment".
    MESSAGE[0] = 2;
    MESSAGE[1] = HANDSHAKE;
    ForwardMessage();
  
    // Wait for a message
    ReceiveMessage();

    // Do whatever with this image segment.
    if(MESSAGE[0] != 5) // look for "Done" message
    {
      // Transmit the message to the designated destination.
      uint8_t destinationAddress = 3;
      messagingLibrary->SendCameraData(MESSAGE, destinationAddress);
    }
    else getAnotherSegment = false; // no more camera data to send
  }
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 2;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 0;
  MESSAGE[1] = 0;
  while((MESSAGE[0] != 2) || (MESSAGE[1] != HANDSHAKE))
  {
    Wait(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  // Get total message length;
  int inputByte = -1;
  uint8_t byteCount = 0;
  while(inputByte == -1) inputByte = Serial.read();
  MESSAGE[0] = (uint8_t)inputByte;
  byteCount++;

  // Forward the message itself
  while(byteCount < MESSAGE[0])
  {
    inputByte = -1;
    while(inputByte == -1) inputByte = Serial.read();
    MESSAGE[byteCount] = (uint8_t)inputByte;
    byteCount++;
  }
}

void ForwardMessage()
{
  // Notice that MESSAGE[0] is the total length of the message,
  // not the number of following bytes. This means the number of
  // following bytes is one less than MESSAGE[0].
  
  Serial.write(MESSAGE[0]); // write one message byte
  Serial.flush();
  uint8_t numBlocks = (uint8_t)((MESSAGE[0] - 1) / SIZE_IO_BUFFER);
  uint8_t thisByte = 1;
  
  for(uint8_t b = 0; b < numBlocks; b++)
  {
    for(uint8_t c = 0; c < SIZE_IO_BUFFER; c++)
    {
      Serial.write(MESSAGE[thisByte++]); // write one message byte
    }
    Serial.flush();
  }
  while(thisByte < MESSAGE[0]) Serial.write(MESSAGE[thisByte++]);
  Serial.flush();
}

void ForwardMessage(String text)
{
  MESSAGE[0] = (uint8_t)text.length() + 1;
  sprintf((char*)(MESSAGE + 1), "%s", text.c_str());
  ForwardMessage();
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void Wait(long milliseconds)
{
  long beginTime = millis();
  uint8_t doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
