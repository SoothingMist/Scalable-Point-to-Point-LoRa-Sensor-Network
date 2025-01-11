
// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = 110; // ascii code
const uint8_t MAX_MESSAGE_LENGTH = 255;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];

// Forward function declarations
void Connect();
void ForwardMessage();
void ReceiveMessage();

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) delay(100); // make sure Serial is ready

  // Wait for connection with external device
  Connect();
  delay(1000);
}

void loop()
{
  // Wait for a message
  ReceiveMessage();

  // Assuming a one-byte message for this application
  MESSAGE[0] = 01;
  MESSAGE[1] = MESSAGE[1] + 01;
  ForwardMessage();
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
