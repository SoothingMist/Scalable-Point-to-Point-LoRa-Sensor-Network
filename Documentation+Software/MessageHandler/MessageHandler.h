
#pragma once

// Required for various Arduino variable types
#include <Arduino.h>

// Required for LoRa and Arduino.
// A modification of Sandeep's library.
// His exact library will not work here.
// Use the version provided.
#include "LoRa.h"

// These constants are set for a given node within a given system.
// There is some indication that they can be made permanently 
// resident on the microcontroller board and queried. 
// That can be done once we learn how.
#define SYSTEM_ID 111

// Messages start with a standard header.
// Byte cell index of header components are given here.
// See MessageContents.html for a detailed explanation.
#define LOCATION_MESSAGE_LENGTH  0
#define LOCATION_SYSTEM_ID       1
#define LOCATION_SOURCE_ID       2
#define LOCATION_DESTINATION_ID  3
#define LOCATION_MESSAGE_ID      4
#define LOCATION_MESSAGE_TYPE    6
#define LOCATION_SENSOR_ID       7
#define LOCATION_REBROADCASTS    8
#define MESSAGE_HEADER_LENGTH    9

class MessageHandler
{

public:

  // Constructor
  MessageHandler(uint8_t nodeAddress);

  // Deconstructor
  ~MessageHandler();

  // Send specific messages to specific destinations.
  bool SendTextMessage(String text, uint8_t destination);

  // Check for incoming messages
  int CheckForIncomingPacket();

  // Get a copy of the MESSAGE pointer
  const uint8_t* getMESSAGE();

  // Relay a message with decrmented rebroadcast counter.
  void RelayMessage();

private:

  uint8_t MAX_MESSAGE_LENGTH = 0; // maximum length of messages
  uint8_t LOCAL_ADDRESS = 0; // unique node address
  uint8_t messageIndex = 0; // cell index in current message vector

  // Outgoing-message counter for this SYSTEM_ID/SOURCE_NODE_ID
  uint16_t sourceMessageID = 0;
  uint8_t sourceMessageID_HighByte = 0;
  uint8_t sourceMessageID_LowByte = 0;

  // Starts a message with its header
  bool StartMessage(uint8_t messageType, uint8_t destination);

  // Broadcasts a fully-formed LoRa packet
  void BroadcastPacket();

  // Check for outside radio signal
  bool CAD();

  // Non-blocking delay for some length of milliseconds
  void Wait(long milliseconds);

  // LoRa packets are composed of overhead and payload (message envelope and contents).
  // Most commonly, the packet is 256 bytes in length.
  // https://www.sciencedirect.com/topics/computer-science/maximum-packet-size
  // Using this calculator, https://avbentem.github.io/airtime-calculator/ttn/us915/222,
  // we see that message length has to be limited to ensure compliance with maximum time for each transmission.
  uint8_t* MESSAGE = NULL;
};
