
// For the Pixy2 camera.
// NOTE, THIS PROGRAM REQUIRES FIRMWARE VERSION 3.0.11 OR GREATER.
// uses Arduino Uno R3 ICSP SPI port. May also work for Uno R4 and Arduino Mega.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:hooking_up_pixy_to_a_microcontroller_-28like_an_arduino-29
#include <Pixy2.h> // Pixy2 library (installed using Tools/Manage Libraries)
Pixy2 pixy; // Pixy2 object 

// Standard C++ input/output
#include <stdio.h>

// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = 110; // ascii code
const uint8_t MAX_MESSAGE_LENGTH = 255;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
uint16_t numCameraRows = 0;
uint16_t numCameraColumns = 0;

// Forward function declarations
void Connect();
void SendMessage();
void ReceiveMessage();

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) delay(100); // make sure Serial is ready

  // Initialize the pixy object.
  // May generate "error: no response" one time.
  // This is fine but has to be accounted for in receiver software.
  pixy.init();
  delay(1000);
 
  // Getting the RGB pixel values requires the 'video' configuration
  pixy.changeProg("video");
  delay(1000); // takes a bit of time for the camera to reconfigure itself
  // Image dimension are typically given in (width, height), (number of colomns, number of rows)
  numCameraColumns = pixy.frameWidth;
  numCameraRows = pixy.frameHeight;

  // Wait for connection with external device
  Connect();
  delay(100);
}

void loop()
{
  // Column and row for pixel selection
  static uint16_t c = 0;
  static uint16_t r = 0;

  // Wait for the next request
  ReceiveMessage();

  // Unpack request - Expecting ascii 0 .. 127 only
  String request = "";
  for(uint8_t i = 1; i < MESSAGE[0] + 1; i++) request += (char)MESSAGE[i];
  String response = request;

  // Respond to request
  response = "";
  if(request.equals("RetrieveCameraDimensions"))
  {
    response = "(" + String(numCameraColumns) + ", " + String(numCameraRows) + ")";
  }
  else if(request.startsWith("GetNextPixelValue")) // GetPixelValues(column, row)
  {
    // Select the next pixel
    c = c % numCameraColumns; c++;
    r = r % numCameraRows; r++;

    // Check values and generate appropriate response
    if((numCameraColumns > c) && (numCameraRows > r))
    {
      uint8_t red, green, blue;
      if (pixy.video.getRGB(c, r, &red, &green, &blue) == 0)
      {
        response = "RGB(" + String(c) + ", " + String(r) + ") = (" +
                    String(red) + ", " + String(green) + ", " + String(blue) + ")";
      }
      else response = "Error: Camera not functional";
    }
    else response = "Error: Pixel request out of bounds";
  }
  else response = "Error: Unrecognized Request: " + request;

  // Send the response
  const char* message = response.c_str();
  MESSAGE[0] = (uint8_t)response.length();
  for(uint8_t i = 0; i < MESSAGE[0]; i++) MESSAGE[i+1] = (uint8_t)message[i];
  SendMessage();

  // Get next request
  delay(100);
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 1;
  MESSAGE[1] = HANDSHAKE;
  SendMessage();

  // Receive handshake
  MESSAGE[0] = 0;
  MESSAGE[1] = 0;
  while((MESSAGE[0] != 1) && (MESSAGE[1] != HANDSHAKE))
  {
    delay(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 1; i < MESSAGE[0] + 1; i++)
  {
    while( ! Serial.available()) delay(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void SendMessage()
{
  for(uint8_t i = 0; i < MESSAGE[0] + 1; i++) Serial.write(MESSAGE[i]);
}
