
#pragma once

// Enables Serial.println diagnostics. Use when running with Serial Monitor.
//#define DEBUG

// Per https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video_api
// The optional saturate argument when set to true (default) will scale all RGB values 
// such that the greatest of the three values (r, g and b) is maximized (saturated) 
// at 255. When set to false, the unmodified RGB values are returned.
#define SATURATE false

// Adds Arduino's capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// Adds ability to read the Pixy2.1 camera.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:start
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
// NOTE, THIS PROGRAM REQUIRES FIRMWARE VERSION 3.0.11 OR GREATER
// Important: The Pixy2 library is shipped with PIXY_DEBUG defined.
// This causes debug messages to be generated. These messages do not match this software's message format.
// This causes all subsequent messages to be rejected. To alleviate this situation, go to TPixy2.h in the 
// Arduino libraries Pixy2 folder. Open that file and comment out Line 23. Then rebuild this software.
// https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:arduino_api#:~:text=Library%20and%20API-,Installing%20the%20Arduino%20Library,file%20that%20you%20just%20downloaded.
#include <SPI.h>
#include <Pixy2.h>

// These constants are set for a given node within a given system.
// There is some indication that they can be made permanently 
// resident on the microcontroller board and queried. 
// That can be done once we learn how.
#define SYSTEM_ID 111
#define SOURCE_NODE_ID 5

// LoRa message content has a maximum length.
// https://www.sciencedirect.com/topics/computer-science/maximum-packet-size#:~:text=LoRa%20offers%20a%20maximum%20packet,be%20found%20in%20%5B5%5D
// We allocate to that size but do not use it all so that it is easier to manage memory.
#define MAX_MESSAGE_INDEX 0255

// Holds message contents.
// See MESSAGE.cpp
extern uint8_t MESSAGE[256];

// Messages start with a standard header.
// Byte locations of header components are given here.
// See MessageContents.html for a detailed explanation.
#define LOCATION_MESSAGE_INDEX   0
#define LOCATION_SYSTEM_ID       1
#define LOCATION_SOURCE_ID       2
#define LOCATION_DESTINATION_ID  3
#define LOCATION_MESSAGE_ID      4
#define LOCATION_MESSAGE_TYPE    6
#define LOCATION_SENSOR_ID       7
#define LOCATION_REBROADCASTS    8
#define MESSAGE_HEADER_LENGTH    9

// Images have depth.
// Ex: An RGB image has depth of three. Grayscale has a depth of one.
//     NirRBB images have a depth of four. Some images have depth of 1000 or more.
// Software assumes a depth of no more than 255 and no less than 1.
// Presently using Pixy2.1 camera, an RGB camera, depth of three.
#define IMAGE_DEPTH 3

// Number of rows and columns to skip at the image's edges.
// For Pixy2.1, each pixel referenced exists within a 5x5 box.
// The referenced pixel is the box' center pixel.
// The delivered pixel value is an average of those five pixels.
// Unable to find a reliable way of changing that.
#define NUMBER_TO_SKIP 2

class MessageConstruction
{

public:

  // Constructor
  MessageConstruction();

  // Send specific messages to specific destinations.
  // 000 = pixel-level data
  bool ComposeMessage_000(uint8_t destination);

private:

  // Nessage parameters
  const uint8_t NumFreeBytes = MAX_MESSAGE_INDEX - MESSAGE_HEADER_LENGTH  + 1;
  uint8_t MessageIndex = 0; // byte possition in current message

  // Outgoing-message counter for this SYSTEM_ID/SOURCE_NODE_ID
  uint16_t sourceMessageID = 0;
  uint8_t sourceMessageID_HighByte = 0;
  uint8_t sourceMessageID_LowByte = 0;

  // Starts a message with its header
  bool StartMessage(uint8_t messageType, uint8_t destination);

   // Forward the message
  bool ForwardMessage();

  // Retrieve the message's ID
  uint16_t GetMessageID();

  // We assume there will be at only one camera.
  // Memory on Uno R3 is insufficient to support more than one.
  Pixy2 pixy2Camera;

  // Camera parameters. Obtained by querying the camera.
  uint16_t ImageHeight = 0;
  uint16_t ImageWidth = 0;
};
