
// Makes use of this debugging technique:
// https://github.com/SoothingMist/Arduino-as-USB-to-Serial-Adaptor.
//#define DEBUG

// For the Pixy2 camera.
// NOTE, THIS PROGRAM REQUIRES FIRMWARE VERSION 3.0.11 OR GREATER.
// uses Arduino Uno R3 ICSP SPI port. May also work for Uno R4 and Arduino Mega.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:hooking_up_pixy_to_a_microcontroller_-28like_an_arduino-29
#include <Pixy2.h> // Pixy2 library (installed using Tools/Manage Libraries)
Pixy2 pixy; // Pixy2 object

// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 222
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
uint8_t SIZE_OUTPUT_BUFFER = 0;
uint16_t numCameraRows = 0;
uint16_t numCameraColumns = 0;

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready
  #ifdef DEBUG
    Serial1.begin(9600); while(! Serial1) Wait(100);
    Serial1.println("Serial port ready");
  #endif

  // Get size of this device's serial output buffer
  // https://www.arduino.cc/reference/tr/language/functions/communication/serial/availableforwrite
  SIZE_OUTPUT_BUFFER = Serial.availableForWrite();

  // Initialize the pixy object.
  // May generate "error: no response" one time.
  // This is fine but has to be accounted for in receiver software.
  pixy.init();
  Wait(1000); // takes a bit of time for the camera to initialize itself
 
  // Getting the RGB pixel values requires the 'video' configuration
  pixy.changeProg("video");
  Wait(1000); // takes a bit of time for the camera to reconfigure itself
  // Image dimension are typically given in (width, height), (number of colomns, number of rows)
  numCameraColumns = pixy.frameWidth;
  numCameraRows = pixy.frameHeight;
  #ifdef DEBUG
    Serial1.println("Pixy ready");
  #endif

  // Wait for connection with external device
  DataConnect();
  #ifdef DEBUG
    Serial1.println("Connection successful");
  #endif
}

void loop()
{
  // Wait for the next request
  ReceiveMessage();

  // Unpack request - Expecting ascii 0 .. 127 only
  String request = (char*)(MESSAGE + 1);

  // Respond to requests
  
  if(request.startsWith("GetEntireImage"))
  {
    for(uint16_t r = 0; r < numCameraRows; r++)
    {
      for(uint16_t c = 0; c < numCameraColumns; c++)
      {
        uint8_t red, green, blue;
        if (pixy.video.getRGB(c, r, &red, &green, &blue) == 0)
        {
          MESSAGE[0] = 8;
          MESSAGE[1] = (uint8_t)((c | 00) >> 8);
          MESSAGE[2] = (uint8_t)((c << 8) >> 8);
          MESSAGE[3] = (uint8_t)((r | 00) >> 8);
          MESSAGE[4] = (uint8_t)((r << 8) >> 8);
          MESSAGE[5] = red;
          MESSAGE[6] = green;
          MESSAGE[7] = blue;
          ForwardMessage();
        }
        else
        {
          ForwardMessage("Error: Camera not functional");
          break;
        }
      }
    }
    ForwardMessage("Done");
  }
  
  else if(request.startsWith("RetrieveCameraDimensions"))
  {
    MESSAGE[0] = 5;
    MESSAGE[1] = (uint8_t)((numCameraColumns | 00) >> 8);
    MESSAGE[2] = (uint8_t)((numCameraColumns << 8) >> 8);
    MESSAGE[3] = (uint8_t)((numCameraRows | 00) >> 8);
    MESSAGE[4] = (uint8_t)((numCameraRows << 8) >> 8);
    ForwardMessage();
  }
  
  else if(request.startsWith("GetPixelValues(")) // GetPixelValues(column, row)
  {
    if(request.endsWith(")"))
    {
      request = request.substring(request.indexOf('(') + 1, request.indexOf(')'));
      int index = request.indexOf(',');
      uint16_t c = request.substring(0, index).toInt();
      uint16_t r = request.substring(index + 1, request.length()).toInt();
      if((numCameraColumns < c) && (numCameraRows < r))
      {
        uint8_t red, green, blue;
        if (pixy.video.getRGB(c, r, &red, &green, &blue) == 0)
        {
          MESSAGE[0] = 8;
          MESSAGE[1] = (uint8_t)((c | 00) >> 8);
          MESSAGE[2] = (uint8_t)((c << 8) >> 8);
          MESSAGE[3] = (uint8_t)((r | 00) >> 8);
          MESSAGE[4] = (uint8_t)((r << 8) >> 8);
          MESSAGE[5] = red;
          MESSAGE[6] = green;
          MESSAGE[7] = blue;
          ForwardMessage();
        }
        else ForwardMessage("Error: Camera not functional");
      }
      else ForwardMessage("Error: Pixel request out of bounds");
    }
    else ForwardMessage("Error: Syntax: Expecting ) at end of " + request);
  }
  else ForwardMessage("Error: Unrecognized Request: " + request);
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void DataConnect()
{
  #ifdef DEBUG
    Serial1.println("Attempting to connect");
  #endif
  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 02) || (MESSAGE[1] != HANDSHAKE))
  {
    Wait(100);
    ReceiveMessage();
  }
  #ifdef DEBUG
    Serial1.println("\tReceived handshake");
  #endif
  
  // Send handshake
  MESSAGE[0] = 02;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();
  #ifdef DEBUG
    Serial1.println("\tSent handshake");
  #endif
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
      Serial1.print("Sening message of length: "); Serial1.println(MESSAGE[0]); Serial1.flush();
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
  uint8_t doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
