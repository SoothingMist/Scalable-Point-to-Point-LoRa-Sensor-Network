
#pragma once

extern void ForwardMessage();
extern void ReceiveMessage();
extern void ForwardMessage(String text);
extern void Wait(long milliseconds);

// Per https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video_api
// The optional saturate argument when set to true (default) will scale all RGB values 
// such that the greatest of the three values (r, g and b) is maximized (saturated) 
// at 255. When set to false, the unmodified RGB values are returned.
#define SATURATE false

#ifdef DEBUG
  // Enables the monitoring of memory utilization.
  // https://github.com/mpflaga/Arduino-MemoryFree
  #include <MemoryFree.h>
#endif

// Adds ability to read the Pixy2.1 camera.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:start
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
// NOTE, THIS PROGRAM REQUIRES FIRMWARE VERSION 3.0.11 OR GREATER
// Important: The Pixy2 library is shipped with PIXY_DEBUG defined.
// This causes debug messages to be generated. These messages do not match this software's message format.
// This causes all subsequent messages to be rejected. To alleviate this situation, go to TPixy2.h in the 
// Arduino libraries Pixy2 folder. Open that file and comment out Line 23. Then rebuild this software.
#include <Pixy2.h>

// Messages start with a standard header. Content follows, up to the maximum message length.
// This value is because of the small memory size of the Arduino Uno R3.
//#define MAX_MESSAGE_LENGTH 63
// This value is due to laws in the USA. Tested with Arduino Mega 2560.
#define MAX_MESSAGE_LENGTH 220
// This value comes from the MessageHandler library.
// That library deals with LoRa transmit/receive.
// We ultimately will do that with messages formed here.
#define MESSAGE_HEADER_LENGTH 9

// To hold incoming and outgoing messages.
// Accommodates first byte being total message length.
unsigned char MESSAGE[MAX_MESSAGE_LENGTH + 1];

// Images have depth.
// Ex: An RGB image has depth of three. Grayscale has a depth of one.
// Software assumes a depth of no more than 255 and no less than 1.
// Presently using Pixy2.1 camera, an RGB camera, depth of three.
#define IMAGE_DEPTH 3

// Number of rows and columns to skip at the image's edges.
// For Pixy2.1, each pixel referenced exists within a 5x5 box.
// The referenced pixel is the box' center pixel.
// The delivered pixel value is an average of those five pixels.
// Unable to find a reliable way of changing that.
#define NUMBER_TO_SKIP 2

// Concerning the message itself.
unsigned char messageIndex = 0; // byte possition in current message
const unsigned char MAX_USABLE_BYTES = MAX_MESSAGE_LENGTH - MESSAGE_HEADER_LENGTH;

// We assume there will be at only one camera.
// Memory on Uno R3 is insufficient to support more than one.
Pixy2 pixy2Camera;

// Camera parameters. Obtained by querying the camera.
unsigned int imageHeight = 0;
unsigned int imageWidth = 0;

// Initializes the camera.
bool InitializeCamera()
{
  // Initialize the camera.
  if(pixy2Camera.init() == PIXY_RESULT_OK)
  {
    pixy2Camera.changeProg("video");
    imageHeight = pixy2Camera.frameHeight;
    imageWidth = pixy2Camera.frameWidth;
    #ifdef DEBUG
      Serial1.println(F("\nPixy2 initialized."));
      Serial1.print(F("Camera Height (Rows): ")); Serial.println(imageHeight);
      Serial1.print(F("Camera Width (Columns): ")); Serial.println(imageWidth);
    #endif
    return true;
  }
  else
  {
    #ifdef DEBUG
      Serial1.println(F("\n*** Camera initialization failed\n"));
      Wait(1000);
    #endif
    return false;
  }
}

// Send all segments of the image's pixels
bool GatherCameraData()
{
  // Take snapshot.
  // Pixy2.1 cannot take static images.
  // Therefore, physically, the camera has to be held in a staring, continuous-dwell, position.
  // Motion in the image or the camera's motion changes the image as this code tries to send the image.
  // The camera is in video mode. It does not deliver a static snapshot.

  // Paints the image row by row.
  // The number of columns is divided by SEGMENT_SIZE,
  // the number of pixels that fit within a message envelope.
  // The standard envelope contains MESSAGE_HEADER_LENGTH bytes.
  // The envelope for message type 000 contains an extra 6 bytes.
  const unsigned char SEGMENT_SIZE = (unsigned char)((MAX_USABLE_BYTES - 6) / IMAGE_DEPTH);

  // For segmenting the image.
  const unsigned char NUM_FULL_COLUMN_BLOCKS =
    (unsigned char)((imageWidth - (2 * NUMBER_TO_SKIP)) / SEGMENT_SIZE); // number of pixels in full column segments
  const unsigned char NUM_REMAINING_PIXELS =
    (unsigned char)((imageWidth - (2 * NUMBER_TO_SKIP)) % SEGMENT_SIZE); // number of pixels in the remaining column partial segment

  // Number of rows and columns can be greater than 255 so we have to get low and high bytes of each.
  // columns change constantly but rows change less often.
  unsigned char row_HighByte;
  unsigned char row_LowByte;

  // Values of image colors.
  unsigned char red, green, blue;

  // Send messages containing pixels on a row-by-row basis.
  // Segments of each row's columns are selected as pixels to add to the message.
  // Each message contains no more than SEGMENT_SIZE number of pixels.
  for (unsigned int r = NUMBER_TO_SKIP; r < imageHeight - NUMBER_TO_SKIP; r++)
  {
    row_HighByte = (unsigned char)(r >> 8);
    row_LowByte = (unsigned char)((r << 8) >> 8);
    unsigned int c = NUMBER_TO_SKIP;

    // Go through each SEGMENT_SIZE block of pixels
    for (unsigned char cb = 0; cb < NUM_FULL_COLUMN_BLOCKS; cb++)
    {
      // Each message contains a block of column pixels for a given row.
      // row and column indicate where a given segment of pixels starts.
      messageIndex = 1;
      MESSAGE[messageIndex++] = row_HighByte;
      MESSAGE[messageIndex++] = row_LowByte;
      MESSAGE[messageIndex++] = (unsigned char)(c >> 8); // column high byte
      MESSAGE[messageIndex++] = (unsigned char)((c << 8) >> 8); // column low byte
      MESSAGE[messageIndex++] = SEGMENT_SIZE;
      MESSAGE[messageIndex++] = IMAGE_DEPTH;

      // Append the pixels in this segment.
      for (unsigned char ss = 0; ss < SEGMENT_SIZE; ss++)
      {
        // Get this pixel's values
        pixy2Camera.video.getRGB(c, r, &red, &green, &blue, SATURATE);
        MESSAGE[messageIndex++] = red;
        MESSAGE[messageIndex++] = green;
        MESSAGE[messageIndex++] = blue;
        
        // Increment the column counter
        c++;
      }

      // Forward the message through the serial port.
      MESSAGE[0] = messageIndex;
      while(Serial.peek() == -1) Wait(100); // wait for request for next segment
      while(Serial.available()) Serial.read(); // do not corrupt the MESSAGE vector
      ForwardMessage();
    }

    // Send the pixels in the last partial segment of columns
    if(NUM_REMAINING_PIXELS > 0)
    {
      // Each message contains a block of column pixels for a given row.
      // row and column indicate where a given segment of pixels starts.
      messageIndex = 1;
      MESSAGE[messageIndex++] = row_HighByte;
      MESSAGE[messageIndex++] = row_LowByte;
      MESSAGE[messageIndex++] = (unsigned char)(c >> 8); // column high byte
      MESSAGE[messageIndex++] = (unsigned char)((c << 8) >> 8); // column low byte
      MESSAGE[messageIndex++] = NUM_REMAINING_PIXELS;
      MESSAGE[messageIndex++] = IMAGE_DEPTH;

      for (c = c; c < imageWidth - NUMBER_TO_SKIP; c++)
      {
        // Get this pixel's values
        pixy2Camera.video.getRGB(c, r, &red, &green, &blue, SATURATE);
        MESSAGE[messageIndex++] = red;
        MESSAGE[messageIndex++] = green;
        MESSAGE[messageIndex++] = blue;
      }
      
      // Forward the message through the serial port.
      MESSAGE[0] = messageIndex;
      while(Serial.peek() == -1) Wait(100); // wait for request for next segment
      while(Serial.available()) Serial.read(); // do not corrupt the MESSAGE vector
      ForwardMessage();
    }
  }
  while(Serial.peek() == -1) Wait(100); // wait for request for next segment
  while(Serial.available()) Serial.read(); // do not corrupt the MESSAGE vector
  ForwardMessage("Done");
}
