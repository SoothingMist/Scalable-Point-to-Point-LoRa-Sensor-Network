
// Enables Serial.println diagnostics. Use when running with Serial Monitor.
//#define DEBUG

#include "MessageHandler.h" // class declaration

// Constructor
MessageHandler::MessageHandler(uint8_t nodeAddress)
{
  // Establish a unique address for this node within the network.
  LOCAL_ADDRESS = nodeAddress;

  // Initialize LoRa transceiver.
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(FREQUENCY))
  {
    #ifdef DEBUG
      Serial.println("Starting LoRa failed!");
      Wait(1000);
    #endif
    exit(1);
  }
  #ifdef DEBUG
   Serial.println("Transceiver is active");
  #endif

  // From calculator using payload size of 222 and overhead of 13.
  // https://avbentem.github.io/airtime-calculator/ttn/us915/222
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.enableCrc(); // rejects corrupted messages without notice
  #ifdef DEBUG
    Serial.print("Frequency: "); Serial.println(FREQUENCY);
    Serial.print("Spreading Factor: "); Serial.println(SPREADING_FACTOR);
    Serial.print("Signal Bandwidth: "); Serial.println(SIGNAL_BANDWIDTH);
    Serial.print("Max message length: "); Serial.println(MAX_MESSAGE_LENGTH);
    Serial.print("Node Address: "); Serial.println(LOCAL_ADDRESS);
  #endif
}

// Deconstructor
MessageHandler::~MessageHandler()
{
}

// Starts a message with its header
bool MessageHandler::StartMessage(uint8_t messageType, uint8_t destination)
{
  // Increment Source message ID.
  sourceMessageID++;
  
  MESSAGE[LOCATION_SYSTEM_ID] = SYSTEM_ID;
  MESSAGE[LOCATION_SOURCE_ID] = LOCAL_ADDRESS;
  MESSAGE[LOCATION_DESTINATION_ID] = destination;
  MESSAGE[LOCATION_MESSAGE_ID] = (uint8_t)(sourceMessageID >> 8); // high byte
  MESSAGE[LOCATION_MESSAGE_ID + 1] = (uint8_t)((sourceMessageID << 8) >> 8); // low byte
  MESSAGE[LOCATION_MESSAGE_TYPE] = messageType;
  MESSAGE[LOCATION_REBROADCASTS] = 05; // number of times to rebroadcast this message
  messageIndex = MESSAGE_HEADER_LENGTH;
  return true;
}

// Send an image segment.
bool MessageHandler::SendCameraData(uint8_t* imageSegment, uint8_t destination)
{
  // Ignore if segment is too long.
  // Should trigger an error message if false is returned.
  if ((MESSAGE_HEADER_LENGTH + imageSegment[0]) > MAX_MESSAGE_LENGTH)
    return false;

  // Start with the message header.
  StartMessage(0, destination); // image-segment messages are type zero

  // Add the segment to the message header.
  MESSAGE[LOCATION_MESSAGE_LENGTH] = MESSAGE_HEADER_LENGTH + imageSegment[0];
  memcpy(MESSAGE + MESSAGE_HEADER_LENGTH, imageSegment + 1, imageSegment[0] - 1);

  // Create a packet containing the message and broadcast.
  BroadcastPacket();
}

// Send text message.
bool MessageHandler::SendTextMessage(String text, uint8_t destination)
{
  // Ignore if text is too long
  if((MESSAGE_HEADER_LENGTH + text.length()) > MAX_MESSAGE_LENGTH)
    return false;

  // Start with the message header.
  StartMessage(3, destination);
  
  // Add the text.
  MESSAGE[LOCATION_MESSAGE_LENGTH] = MESSAGE_HEADER_LENGTH + text.length();
  memcpy(MESSAGE + MESSAGE_HEADER_LENGTH, (uint8_t*)text.c_str(), text.length());

  // Create a packet containing the message and broadcast.
  BroadcastPacket();
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void MessageHandler::Wait(long milliseconds)
{
  long beginTime = millis();
  uint8_t doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}

// Send a packet.
void MessageHandler::BroadcastPacket()
{
  // Try next channel activity detection.
  // Transmit if no signal detected.
  while (LoRa.rxSignalDetected()) Wait(100); // wait for clear channel
  LoRa.beginPacket();                        // start packet
  LoRa.write(MESSAGE, MESSAGE[LOCATION_MESSAGE_LENGTH]); // add contents
  LoRa.endPacket();                          // finish packet and send it
  #ifdef DEBUG
        Serial.print("Sent message of length "); Serial.println(MESSAGE[LOCATION_MESSAGE_LENGTH]);
  #endif
}

// Look for an incoming packet. Parse if present.
// Contents exists in MESSAGE if packet for this node.
// Returns:  0 if no message present
//          -1 if message present but not for this node
//          >0 if message present and for this node
int MessageHandler::CheckForIncomingPacket()
{
  // actually not the size of the whole packet, just the message contents
  int messageSize = LoRa.parsePacket();

  // download the message contents
  if(messageSize > 0)
  {
    if(messageSize < MESSAGE_HEADER_LENGTH ||
       messageSize > MAX_MESSAGE_LENGTH)
    {
      for(int i = 0; i < messageSize; i++) LoRa.read();
      return -1;
    }
    
    for(int i = 0; i < MESSAGE_HEADER_LENGTH; i++)
      MESSAGE[i] = LoRa.read();

    // if the message is not for this node, ignore
    if (MESSAGE[LOCATION_DESTINATION_ID] != LOCAL_ADDRESS &&
        LOCAL_ADDRESS != 00) // relays have address 0
    {
      #ifdef DEBUG
        Serial.println("This message is not for me.");
      #endif
    
      for(int i = MESSAGE_HEADER_LENGTH; i < messageSize; i++) LoRa.read();
      return -1;
    }

    // message is for this node

    for(int i = MESSAGE_HEADER_LENGTH; i < messageSize; i++)
      MESSAGE[i] = LoRa.read();

    #ifdef DEBUG
      Serial.println("Received from: 0x" + String(MESSAGE[LOCATION_SOURCE_ID], HEX));
      Serial.println("Sent to: 0x" + String(MESSAGE[LOCATION_DESTINATION_ID], HEX));
      Serial.println("Message length: " + String(messageSize));
      Serial.print("RSSI: " + String(LoRa.packetRssi()));
      Serial.println("   Snr: " + String(LoRa.packetSnr()));
      Serial.println();
    #endif
  }
  
  return messageSize;
}

const uint8_t* MessageHandler::getMESSAGE() { return (const uint8_t*)MESSAGE; }
void MessageHandler::RelayMessage() { MESSAGE[LOCATION_REBROADCASTS] -= 1; BroadcastPacket(); }
