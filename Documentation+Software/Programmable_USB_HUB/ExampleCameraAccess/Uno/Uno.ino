
// For the Pixy2 camera.
// NOTE, THIS PROGRAM REQUIRES FIRMWARE VERSION 3.0.11 OR GREATER.
// uses Arduino Uno R3 ICSP SPI port. May also work for Uno R4 and Arduino Mega.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:hooking_up_pixy_to_a_microcontroller_-28like_an_arduino-29
#include <Pixy2.h> // Pixy2 library (installed using Tools/Manage Libraries)
Pixy2 pixy; // Pixy2 object 

// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 63
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
uint16_t numCameraRows = 0;
uint16_t numCameraColumns = 0;

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready

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

  // Wait for connection with external device
  Connect();
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
          Wait(1000); // messaging gets corrupted otherwise, trying to avoid handshaking between messages
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
  while( ! Serial.available()) Wait(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 1; i < MESSAGE[0]; i++)
  {
    while( ! Serial.available()) Wait(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 0; i < MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
}

void ForwardMessage(String text)
{
  text = "* " + text;
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
