
#pragma once

// Enables Serial.println diagnostics. Use when running with Serial Monitor.
//#define DEBUG

// Adds Arduino's capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// These constants are set for a given node within a given system.
// There is some indication that they can be made permanently 
// resident on the microcontroller board and queried. 
// That can be done once we learn how.
#define SYSTEM_ID 111
#define NODE_ID 2

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

// Given the limited memory on relay nodes, 
// they can only keep track of a limted number of sender nodes.
// Choose MAX_NODES accordingly.
// Node IDs are assumed to be 1 .. (MAX_NODES -1), inclusive.
// All relay nodes use ID = 0 since they do not originate messages.
#define MAX_NODES 025

class MessageHandling
{

public:

  // Constructor
  MessageHandling();

  // Rebroadcast as needed
  bool CheckForPassthrough();

private:

  // List of node IDs and their maximum message ID seen so far.
  // Message IDs are assigned in sequence by the originating node.
  uint16_t LatestMessageID[MAX_NODES];

  // Retrivals
  uint16_t GetMessageID();
};
