
// In case an OLED has been installed
//#define OLED

#ifdef OLED
  // Required for the RAK1921 OLED
  #include <U8g2lib.h>	// Click to install library: http://librarymanager/All#u8g2
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
  void Disply_On_RAK1921(int x, int y, const char* displayText);
#endif

// Constants and variables used by various subroutines
const uint8_t HANDSHAKE = 110; // ascii code
const uint8_t MAX_MESSAGE_LENGTH = 255;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH];

// Forward function declarations
void Connect();
void SendMessage();
void ReceiveMessage();

void setup()
{
  // Initialize Arduino
  Serial.begin(9600);
  while(!Serial) delay(100); // make sure Serial is ready

  #ifdef OLED
    // Intialize the RAK1921 OLED
    u8g2.begin();
    delay(100);
  #endif

  // Wait for connection with external device
  Connect();
  delay(100);
}

void loop()
{
  // Send request
  //String request = "RetrieveCameraDimensions"; // ascii 0 .. 127 only
  String request = "GetNextPixelValue"; // ascii 0 .. 127 only
  const char* message = request.c_str();
  MESSAGE[0] = request.length();
  for(int i = 0; i < MESSAGE[0]; i++) MESSAGE[i+1] = message[i];
  SendMessage();

  // Receive the response
  String response = "";
  ReceiveMessage();
  for(int i = 1; i < MESSAGE[0] + 1; i++) response += (char)MESSAGE[i];

  // Do something with the response

  #ifdef OLED
    // Display response on OLED
    Disply_On_RAK1921(3, 15, response.c_str());
  #endif

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

#ifdef OLED
  void Disply_On_RAK1921(int x, int y, const char* displayText)
  {
    u8g2.clearBuffer();	// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB10_tr); // choose a suitable font
    u8g2.drawStr(x, y, displayText);
    u8g2.sendBuffer(); // transfer internal memory to the display
  }
#endif
