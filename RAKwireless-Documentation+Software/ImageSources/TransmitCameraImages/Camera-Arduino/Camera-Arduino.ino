
// Adds ability to pack and transmit various message types.
// Initializes Pixy camera.
// Contains DEBUG definition.
#include "MessageConstruction.h"
MessageConstruction *MessageConstructor = NULL;

// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = 110; // ascii code

// Destination for messages.
// This could eventually be a variable.
#define DESTINATION_NODE 1

// Holds message contents.
// See MESSAGE.cpp
extern uint8_t MESSAGE[256];

// Forward function declarations
void Connect();
void ReceiveMessage();
void ForwardMessage();

// Microcontroller setup code. Runs once.
void setup()
{
  // Initialize serial port
  // https://docs.arduino.cc/language-reference/en/functions/communication/serial/begin
  // Baudrate = 9600, data bits = 8, parity = none, stop bits = 1
  Serial.begin(9600, SERIAL_8N1);

  unsigned long startTime = millis();
  while (!Serial)
  {
    if ((millis() - startTime) < 5000) delay(100);
    else break;
  }

  #ifdef DEBUG
    Serial.println("\nSerial Port Initialized.");
  #endif

  // Create one message constructor.
  MessageConstructor = new MessageConstruction;

  // Wait for connection with external device
  #ifdef DEBUG
    Serial.println("setup() completed.");
  #else
    Connect(); // connect to external device vis serial port
  #endif

  // Wait a bit for device to "settle".
  //startTime = millis();
  //while(millis() - startTime < 100);
}

// Runs repeatedly on the microcontroller.
void loop()
{
  #ifdef DEBUG
    // Wait for picture to be requested.
    // Notice that messages can be no longer than 255 bytes and contain at least one byte.
    String message = "Camera Ready. Awaiting one-character request.\n";
    Serial.write(message.c_str(), message.length()); // Send "ready" message
    while (Serial.available() < 1); // await input
    Serial.read(); // read one byte
    while (Serial.available() > 0) Serial.read(); // flushes all other incoming data
  #endif

  // Send an image.
  MessageConstructor->ComposeMessage_000(DESTINATION_NODE);
exit(0);
  // Wait a bit before sending another image
  unsigned long startTime = millis();
  while(millis() - startTime < 3000);
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 01;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 01) && (MESSAGE[1] != HANDSHAKE))
  {
    delay(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 01; i <= MESSAGE[0]; i++)
  {
    while( ! Serial.available()) delay(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 00; i <= MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
}
