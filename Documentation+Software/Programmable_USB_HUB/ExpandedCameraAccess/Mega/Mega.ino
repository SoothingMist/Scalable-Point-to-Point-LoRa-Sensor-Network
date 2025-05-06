
// Makes use of this debugging technique:
// https://github.com/SoothingMist/Arduino-as-USB-to-Serial-Adaptor.
//#define DEBUG

// Adds ability to pack and forward camera data.
// Initializes Pixy camera.
#include "CameraData.h"

// Constants and variables used by various subroutines
// Constants and variables regarding messages.
const uint8_t HANDSHAKE = (uint8_t)'H';
uint8_t SIZE_OUTPUT_BUFFER = 0;

void setup()
{
  // Initialize Arduino
  Serial.begin(9600); while(!Serial) Wait(100);
  #ifdef DEBUG
    Serial1.begin(9600); while(! Serial1) Wait(100);
  #endif
  
  // Get size of this device's serial output buffer
  // https://www.arduino.cc/reference/tr/language/functions/communication/serial/availableforwrite
  SIZE_OUTPUT_BUFFER = Serial.availableForWrite();

  // Wait for connection with external device
  Connect();
  #ifdef DEBUG
    Serial1.print("Respondent ready\n"); Serial1.flush();
  #endif
  
  // Initialize the camera.
  // Could send an error message here.
  if( ! InitializeCamera()) exit(1);
}

void loop()
{
  // Wait for picture to be requested.
  #ifdef DEBUG
    Serial1.print("Waiting to receive message\n"); Serial1.flush();
  #endif
  ReceiveMessage();

  // Assuming for now that the request is for
  // an entire image to be sent.
  GatherCameraData();
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
  #ifdef DEBUG
    Serial1.print("Read message of length "); Serial1.println(MESSAGE[0]); Serial1.flush();
  #endif
}

void ForwardMessage()
{
  // Notice that MESSAGE[0] is the total length of the message,
  // not the number of following bytes. This means the number of
  // following bytes is one less than MESSAGE[0].

  #ifdef DEBUG
    Serial1.print("Message Length: "); Serial1.println(MESSAGE[0]); Serial1.flush();
  #endif
  Serial.write(MESSAGE[0]); // write one message byte
  Serial.flush();
  uint8_t numBlocks = (uint8_t)((MESSAGE[0] - 1) / SIZE_OUTPUT_BUFFER);
  uint8_t thisByte = 1;
  
  for(uint8_t b = 0; b < numBlocks; b++)
  {
    for(uint8_t c = 0; c < SIZE_OUTPUT_BUFFER; c++)
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
  uint8_t doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
