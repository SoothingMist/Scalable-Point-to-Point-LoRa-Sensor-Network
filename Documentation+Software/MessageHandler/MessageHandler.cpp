
// Enables Serial.println diagnostics. Use when running with Serial Monitor.
#define DEBUG

#include "MessageHandler.h" // class declaration

// Could not find a better way to host this callback routine.
// Flags signal detection during CAD process
uint8_t signalDetectionFlag = 00;
// Callback for CAD process.
void onCadDone(boolean signalDetected)
{
  signalDetectionFlag = (uint8_t)signalDetected;
}

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
  #ifdef DEBUG
    Serial.print("Frequency: "); Serial.println(FREQUENCY);
    Serial.print("Spreading Factor: "); Serial.println(SPREADING_FACTOR);
    Serial.print("Signal Bandwidth: "); Serial.println(SIGNAL_BANDWIDTH);
    Serial.print("Max message length: "); Serial.println(MAX_MESSAGE_LENGTH);
    Serial.print("Node Address: "); Serial.println(LOCAL_ADDRESS);
  #endif

  // Register the channel activity dectection callback
  LoRa.onCadDone(onCadDone);
}

// Deconstructor
MessageHandler::~MessageHandler()
{
}

// Starts a message with its header
bool MessageHandler::StartMessage(uint8_t messageType, uint8_t destination)
{
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

// Send text message.
bool MessageHandler::SendTextMessage(String text, uint8_t destination)
{
  // Ignore if text is too long
  if((MESSAGE_HEADER_LENGTH + text.length()) > MAX_MESSAGE_LENGTH)
    return false;

  // Increment Source message ID.
  sourceMessageID++;
  
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
  // try next activity detection.
  // transmit if no signal detected.
  signalDetectionFlag = 01; // assume there is a detection
  while (signalDetectionFlag)
  {
    signalDetectionFlag = 02;
    LoRa.channelActivityDetection();
    while (signalDetectionFlag == 02) Wait(100);
    if (!signalDetectionFlag) // no signal detected
    {
      LoRa.beginPacket(); // start packet
      LoRa.write(MESSAGE, MESSAGE[LOCATION_MESSAGE_LENGTH]); // add contents
      LoRa.endPacket(); // finish packet and send it
      #ifdef DEBUG
        Serial.print("Sent message of length "); Serial.println(MESSAGE[LOCATION_MESSAGE_LENGTH]);
      #endif
    }
  }
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
