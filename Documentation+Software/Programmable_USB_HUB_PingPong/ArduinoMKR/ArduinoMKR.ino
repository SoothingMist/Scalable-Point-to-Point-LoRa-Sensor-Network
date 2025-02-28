
// Constants and variables used by various subroutines
#define HANDSHAKE "H"
uint8_t MESSAGE[256]; // LoRa max message length

// Forward function declarations
void Connect();
void ForwardMessage();
void ReceiveMessage();
void Wait(unsigned long milliseconds);

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready

  // Wait for connection with external device
  Connect();

  // Send instigator message
  ForwardMessage((String)(char)1);
}

void loop()
{
  // Wait for a message
  ReceiveMessage();

  // Assuming a one-byte message for this application
  MESSAGE[1]++;
  ForwardMessage((String)(char)MESSAGE[1]);
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  ForwardMessage(HANDSHAKE);

  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 01) && (MESSAGE[1] != (uint8_t)HANDSHAKE[0]))
  {
    Wait(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while(Serial.available() <= 0) Wait(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 01; i <= MESSAGE[0]; i++)
  {
    while( ! Serial.available()) Wait(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage(String thisString)
{
  const uint8_t* message = (const uint8_t*)thisString.c_str();
  uint8_t messageLength = thisString.length();
  Serial.write(messageLength);
  for(uint8_t i = 00; i < messageLength; i++) Serial.write(message[i]);
}

void Wait(unsigned long milliseconds) // delay() is blocking so we do not use that
{
  unsigned long beginTime = millis();
  while ((millis() - beginTime) < milliseconds);
}
