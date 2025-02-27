
// Enables Serial.println diagnostics. Use when running with Serial Monitor.
#define DEBUG

#include "MessageHandler.h" // class declaration

// Constructor
MessageHandler::MessageHandler(uint8_t nodeAddress = 00)
{
  // Establish a unique address for this node within the network.
  LOCAL_ADDRESS = nodeAddress;

  // Initialize LoRa transceiver.
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(915E6))
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
  MAX_MESSAGE_LENGTH = 0222;
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  #ifdef DEBUG
    Serial.print("Frequency: "); Serial.println(LoRa.getFrequency());
    Serial.print("Spreading Factor: "); Serial.println(LoRa.getSpreadingFactor());
    Serial.print("Signal Bandwidth: "); Serial.println(LoRa.getSignalBandwidth());
    Serial.print("Node Address: "); Serial.println(LOCAL_ADDRESS);
  #endif

  // Establis the vector that holds message data.
  MESSAGE = new uint8_t[MAX_MESSAGE_LENGTH];
}

// Deconstructor
MessageHandler::~MessageHandler()
{
  delete[] MESSAGE;
  MESSAGE = NULL;
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

bool MessageHandler::CAD()
{
  // CAD not fully implemented by Sandeep library.
  bool signalDetected = LoRa.rxSignalDetected();
  
  if (signalDetected)
  {
    Serial.println("Signal detected");
  }
  else
  {
    Serial.println("No signal detected. Could send something.");
  }
  return signalDetected;
}

// Send a packet.
void MessageHandler::BroadcastPacket()
{
  while(CAD()) Wait(100);                // wait for clear channel
  LoRa.beginPacket();                    // start packet
  LoRa.write(MESSAGE, MESSAGE[LOCATION_MESSAGE_LENGTH]); // add contents
  LoRa.endPacket();                      // finish packet and send it
  #ifdef DEBUG
    Serial.print("Sent message of length "); Serial.println(MESSAGE[LOCATION_MESSAGE_LENGTH]);
  #endif
}

void MessageHandler::Wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
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

    for(int i = MESSAGE_HEADER_LENGTH; i < messageSize; i++)
      MESSAGE[i] = LoRa.read();

    #ifdef DEBUG
      // if message is for this device, show details
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
