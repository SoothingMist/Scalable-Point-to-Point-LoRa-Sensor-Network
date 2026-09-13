
// Makes requests of associated devices.
// Receives and parses responses.

// Global constants and variables regarding messages.
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 63
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
String request = "";

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready
  
  // Wait for connection with external device
  TransceiverConnect();

  // Ask for entire image to be sent.
  // Comment this out if requests appear within the loop.
  request = "GetEntireImage";
  ForwardMessage(request);
}

void loop()
{
  // Send request
  //String request = "RetrieveCameraDimensions";
  //String request = "GetPixelValues(0, 0)"; // can be any (column,row) within camera dimensions
  //ForwardMessage(request);

  // Receive the response
  ReceiveMessage();

  // Parse the message
  
  if(request.startsWith("RetrieveCameraDimensions"))
  {
    // do whatever needs done
  }
  
  else if(request.startsWith("GetPixelValues"))
  {
    // do whatever needs done
  }
  
  else if(request.startsWith("GetEntireImage"))
  {
    // do whatever needs done
  }
  
  else // message contents not recognized
  {
    // do whatever needs done
  }
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void TransceiverConnect()
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
