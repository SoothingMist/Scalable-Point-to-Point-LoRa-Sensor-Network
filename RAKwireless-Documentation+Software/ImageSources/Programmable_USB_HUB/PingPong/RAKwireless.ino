
// In case an OLED has been installed
//#define OLED

// Can use built-in LEDs for basic debugging when communicating with another device
//#define LED

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
void ForwardMessage();
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
    u8g2.clearBuffer();	// clear the internal memory
    u8g2.setFont(u8g2_font_ncenB10_tr); // choose a suitable font
  #endif

  #ifdef LED
    // Prepare LED's
    //pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    //digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
  #endif

  // Wait for connection with external device
  Connect();

  // Send instigator message
  MESSAGE[0] = 1;
  MESSAGE[1] = 0;
  ForwardMessage();
}

void loop()
{
  // Wait for a message
  #ifdef LED
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, HIGH);
  #endif

  ReceiveMessage();

  #ifdef LED
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, LOW);
  #endif

  // Assuming a one-byte message for this application
  MESSAGE[0] = 1;
  MESSAGE[1] = MESSAGE[1] + 1;
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
  while(Serial.available() <= 0) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size

  #ifdef OLED
    // Display value sent on OLED
    Disply_On_RAK1921(3, 15, String(MESSAGE[0]).c_str());
  #endif

  for(uint8_t i = 1; i <= MESSAGE[0]; i++)
  {
    //int counter = 0;
    while(Serial.available() <= 0)
    {
      //Disply_On_RAK1921(3, 30, ("Counter: " + String(counter++)).c_str());
      delay(1000); // wait for message
    }
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 00; i <= MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
}

#ifdef OLED
  void Disply_On_RAK1921(int x, int y, const char* displayText)
  {
    //u8g2.clearBuffer();	// clear the internal memory
    //u8g2.setFont(u8g2_font_ncenB10_tr); // choose a suitable font
    u8g2.drawStr(x, y, displayText);
    u8g2.sendBuffer(); // transfer internal memory to the display
  }
#endif
