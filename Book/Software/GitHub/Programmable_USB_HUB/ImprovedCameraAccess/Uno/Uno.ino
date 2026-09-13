
// When the device is running by itself
// and not connected to anything but the Serial Monitor.
//#define DEBUG

// Adds ability to pack and forward camera data.
// Initializes Pixy camera.
#include "CameraData.h"

// Expected handshake during Connect().
const unsigned char HANDSHAKE = (unsigned char)'H';

void setup()
{
  // Open the serial port.
  Serial.begin(9600);
  while (!Serial) Wait(100);

  #ifndef DEBUG
    // Wait for connection with external device
    DataConnect();
  #else
    // Flush the serial port's incoming-data buffer.
    while (Serial.available() > 0) Serial.read();
  #endif

  #ifdef DEBUG
    Serial.println(F("\nSerial Port Initialized.")); Serial.flush(); // flush all outgoing data
  #endif

  // Initialize the camera.
  if( ! InitializeCamera()) exit(1);

  #ifdef DEBUG
    Serial.println(F("setup() completed.")); Serial.flush(); // flush all outgoing data
  #endif
}

// Runs repeatedly on the microcontroller.
void loop()
{
  // Wait for picture to be requested.
  // Notice that message can be no longer than 256 bytes and must contain at least one byte.
  #ifdef DEBUG
    String message = "Camera Ready. Awaiting one-character request.\n";
    Serial.write(message.c_str(), message.length()); // Send "ready" message
  #endif
  while (Serial.available() == 0) Wait(100);    // await input
  unsigned char incomingByte = Serial.read();   // read one byte:
  while (Serial.available() > 0) Serial.read(); // flushes all other incoming data

  // Forward an image through the serial port.
  GatherCameraData();
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void DataConnect()
{
  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 02) || (MESSAGE[1] != HANDSHAKE))
  {
    Wait(100);
    ReceiveMessage();
  }
  
  // Send handshake
  MESSAGE[0] = 02;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();
}

void ReceiveMessage()
{
  while(Serial.available() < 1) Wait(100); // wait for message
  MESSAGE[0] = (unsigned char)Serial.read(); // get message size
  for(unsigned char i = 1; i < MESSAGE[0]; i++)
  {
    while(Serial.available() < 1) Wait(100); // wait for message
    MESSAGE[i] = (unsigned char)Serial.read(); // read message
  }
}

void ForwardMessage(String text)
{
  MESSAGE[0] = (unsigned char)text.length() + 1;
  sprintf((char*)(MESSAGE + 1), "%s", text.c_str());
  ForwardMessage();
}
