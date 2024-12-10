
#pragma once

// Enables Serial.println diagnostics. Use when running with Serial Monitor.
#define DEBUG

// Adds Arduino's capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// These constants are set for a given node within a given system.
// There is some indication that they can be made permanently 
// resident on the microcontroller board and queried. 
// That can be done once we learn how.
#define SYSTEM_ID 111
#define SOURCE_NODE_ID 3

// LoRa message content has a maximum length, 256 bytes.
// https://www.sciencedirect.com/topics/computer-science/maximum-packet-size#:~:text=LoRa%20offers%20a%20maximum%20packet,be%20found%20in%20%5B5%5D
// Thus. the maximum cell index in a message vector is 255.
#define MAX_MESSAGE_INDEX 0255

// Holds message contents.
// See MESSAGE.cpp
extern uint8_t MESSAGE[256];

// Messages start with a standard header.
// Byte cell index of header components are given here.
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

class MessageConstruction
{

public:

  // Constructor
  MessageConstruction();

  // Send specific messages to specific destinations.
  // 003 - Send text message
  bool ComposeMessage_003(uint8_t destination, char* thisText);

private:

  // Nessage parameters
  const uint8_t NumFreeBytes = MAX_MESSAGE_INDEX - MESSAGE_HEADER_LENGTH  + 1;
  uint8_t MessageIndex = 0; // cell index in current message vector

  // Outgoing-message counter for this SYSTEM_ID/SOURCE_NODE_ID
  uint16_t sourceMessageID = 0;
  uint8_t sourceMessageID_HighByte = 0;
  uint8_t sourceMessageID_LowByte = 0;

  // Starts a message with its header
  bool StartMessage(uint8_t messageType, uint8_t destination);
};
