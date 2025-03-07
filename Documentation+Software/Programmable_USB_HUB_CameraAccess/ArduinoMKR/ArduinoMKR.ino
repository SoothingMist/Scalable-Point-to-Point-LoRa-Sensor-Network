
// Makes requests of associated devices.
// Receives and parses responses.

// Constants and variables regarding messages.
const uint8_t HANDSHAKE = (uint8_t)'H';
#define MAX_MESSAGE_LENGTH 256
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];
String request = "";

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) Wait(100); // make sure Serial is ready
  
  // Wait for connection with external device
  Connect();

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
    // Message Format:
    // Byte 0 number of bytes total in message
    // Byte 1 high-byte of pixel x coordinate
    // Byte 2 low-byte of pixel x coordinate
    // Byte 3 high-byte of pixel y coordinate
    // Byte 4 low-byte of pixel y coordinate
    // Byte 5 Red value
    // Byte 6 Green value
    // Byte 7 Blue value
    /*
    // Example of message parsing.
    uint16_t x_coord = MESSAGE[1];
    x_coord = (x_coord << 8) | MESSAGE[2];
    uint16_t y_coord = MESSAGE[3];
    y_coord = (y_coord << 8) | MESSAGE[4];
    Serial.print(F("x_coord ")); Serial.print(x_coord);
    Serial.print(F("  y_coord ")); Serial.println(y_coord);
    Serial.print(F("Red ")); Serial.print(MESSAGE[5]);
    Serial.print(F("  Green ")); Serial.print(MESSAGE[6]);
    Serial.print(F("  Blue ")); Serial.print(MESSAGE[7]);
    Serial.println('\n');
    */

    // do whatever needs done
  }
  
  else // message contents not recognized
  {
    //Serial.print(F("* Error: Unrecognized Request: ")); Serial.println(request);

    // do whatever needs done
  }
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 02;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 02) || (MESSAGE[1] != HANDSHAKE))
  {
    Wait(100);
    ReceiveMessage();
  }
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
