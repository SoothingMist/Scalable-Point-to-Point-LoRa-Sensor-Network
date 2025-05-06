
// Global constants and variables used by various subroutines.
// Note: Arduino serial I/O buffers are only 63 bytes long.
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 63
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
long startTime = 0;
long interval = 0;
uint8_t counter = 0;

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready

  // Wait for connection with external device
  Connect();

  // Initialize time before sending next message
  randomSeed(7); // different random seed for each
  interval = random(1000, 5001); // 1 through 5 seconds
  startTime = millis();
}

void loop()
{
  // If a message is waiting, read it.
  if(Serial.available() > 0) ReceiveMessage();

  // If it is time to send a message, send it.
  if(millis() - startTime > interval)
  {
    ForwardMessage("Message from MKR: " + String(counter++));
    interval = random(1000, 5001); // 1 through 5 seconds
    startTime = millis();
  }
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 2;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 0;
  MESSAGE[1] = 0;
  while((MESSAGE[0] != 2) || (MESSAGE[1] != HANDSHAKE))
  {
    Wait(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) Wait(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 1; i < MESSAGE[0]; i++)
  {
    while( ! Serial.available()) Wait(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 0; i < MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
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
