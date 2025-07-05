
#pragma once

// Check for appropriate board.
#ifdef ARDUINO_SAMD_MKRWAN1300
#error "This program is not compatible with the Arduino MKR WAN 1300 board!"
#endif

// Required for LoRa functionality.
// Based on Sandeep's library from 
// https://github.com/sandeepmistry/arduino-LoRa
// Use the version of the library included with the project.
#include <LoRa.h> // includes Arduino.h

// These constants are set for a given node within a given system.
// There is some indication that they can be made permanently 
// resident on the microcontroller board and queried. 
// That can be done once we learn how.
#define SYSTEM_ID 111

// Transceiver configuration.
// LoRa packets are composed of overhead and payload (envelope and contents).
// Most commonly, the packet is 256 bytes in length.
// https://www.sciencedirect.com/topics/computer-science/maximum-packet-size
// Using this calculator, https://avbentem.github.io/airtime-calculator/ttn/us915/222,
// we see that message length has to be limited to ensure compliance with maximum time for each transmission.
// Spreading factor and signal bandwidth are set accordingly.
// These factors are for a maximum message (payload) length of 222 bytes.
#define FREQUENCY 915E6
#define SPREADING_FACTOR 7
#define SIGNAL_BANDWIDTH 125E3
#define MAX_MESSAGE_LENGTH 222

// Messages start with a standard header.
// Message index of header components are given here.
// Each cell of the message vector is 8 bits in size (uint8_t).
// Note: Other terminology refers to "payload".
//       That is the same as our "message".
//       We form messages and send them via LoRa.
//       The transceiver encloses the message in a packet
//       and broadcasts the packet. Reception takes in
//       a packet and extracts the message.
#define LOCATION_MESSAGE_LENGTH  0
#define LOCATION_SYSTEM_ID       1
#define LOCATION_SOURCE_ID       2
#define LOCATION_DESTINATION_ID  3
#define LOCATION_MESSAGE_ID      4
#define LOCATION_MESSAGE_TYPE    6
#define LOCATION_APPARATUS_ID    7
#define LOCATION_REBROADCASTS    8
#define MESSAGE_HEADER_LENGTH    9

class LoRaMessageHandler
{

public:

  // Constructor
  LoRaMessageHandler(uint8_t nodeAddress);

  // Deconstructor
  ~LoRaMessageHandler();

  // Send specific messages to specific destinations.
  bool SendTextMessage(String text, uint8_t destination);
  bool SendCameraData(uint8_t* imageSegment, uint8_t destination);
  bool SendRequest(uint8_t apparatus, uint32_t associatedValue, uint8_t destination);
  bool SendResponse(uint8_t apparatus, uint32_t associatedValue, uint8_t destination);
  
  // Check for incoming messages
  int CheckForIncomingPacket();

  // Get a copy of the MESSAGE pointer
  const uint8_t* getMESSAGE();

  // Relay a message with decrmented rebroadcast counter.
  void RelayMessage();

  // Non-blocking delay for some length of milliseconds
  void Wait(long milliseconds);

private:

  uint8_t LOCAL_ADDRESS = 0; // unique node address
  uint8_t messageIndex = 0; // cell index in current message vector

  // Outgoing-message counter for this SYSTEM_ID/SOURCE_NODE_ID
  uint16_t sourceMessageID = 0;
  uint8_t sourceMessageID_HighByte = 0;
  uint8_t sourceMessageID_LowByte = 0;

  // Starts a message with its header
  bool StartMessage(uint8_t messageType, uint8_t destination);

  // Broadcasts a fully-formed LoRa packet
  bool BroadcastPacket();

  // Holds the message to be sent.
  // Also holds received messages.
  uint8_t MESSAGE[256]; // never longer
};
