
// Constants and variables regarding messages
const uint8_t HANDSHAKE = 110; // ascii code
const uint8_t MAX_MESSAGE_LENGTH = 255;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH]; // first byte is always message length


// Forward function declarations
void Connect();
void SendMessage();
void ReceiveMessage();

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while(!Serial) delay(100); // make sure Serial is ready

  // Wait for connection with external device
  Connect();
  delay(100);
}

void loop()
{
  // Send request
  //String request = "RetrieveCameraDimensions"; // ascii 0 .. 127 only
  String request = "GetPixelValues(0, 0)"; // ascii 0 .. 127 only
  const char* message = request.c_str();
  MESSAGE[0] = request.length();
  for(int i = 0; i < MESSAGE[0]; i++) MESSAGE[i+1] = message[i];
  SendMessage();

  // Receive the response
  String response = "";
  ReceiveMessage();
  for(int i = 1; i < MESSAGE[0] + 1; i++) response += (char)MESSAGE[i];

  // Make additional requests as needed.
  delay(1000);
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 1;
  MESSAGE[1] = HANDSHAKE;
  SendMessage();

  // Receive handshake
  MESSAGE[0] = 0;
  MESSAGE[1] = 0;
  while((MESSAGE[0] != 1) && (MESSAGE[1] != HANDSHAKE))
  {
    delay(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 1; i < MESSAGE[0] + 1; i++)
  {
    while( ! Serial.available()) delay(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void SendMessage()
{
  for(uint8_t i = 0; i < MESSAGE[0] + 1; i++) Serial.write(MESSAGE[i]);
}
