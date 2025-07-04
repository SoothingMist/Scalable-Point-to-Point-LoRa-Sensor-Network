
// Sensor/DataGenerator side of programmable USB hub.
// Sends data upon request.

// Global constants and variables used by various subroutines
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 256
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready

  // Wait for connection with external device
  DataConnect();
}

void loop()
{
  // Wait for a message request
  ReceiveMessage();

  // Increment the second byte and forward
  MESSAGE[1]++;
  ForwardMessage();
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
  while( ! Serial.available()) Wait(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 01; i < MESSAGE[0]; i++)
  {
    while( ! Serial.available()) Wait(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 00; i < MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
}

void ForwardMessage(String text)
{
  MESSAGE[0] = (uint8_t)text.length() + 1;
  sprintf((char*)(MESSAGE + 1), "%s", text.c_str());
  ForwardMessage();
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void Wait(long milliseconds)
{
  long beginTime = millis();
  uint8_t doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
