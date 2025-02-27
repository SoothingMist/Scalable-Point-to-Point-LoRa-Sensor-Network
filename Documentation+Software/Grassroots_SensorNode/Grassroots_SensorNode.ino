
// Grassroots Sensor Node.
// Periodically broadcasts its sensor value.

// For viewing Serial.println text on serial monitor.
#define DEBUG

// Unique address of this network node
#define localAddress 01

// Library for message handling.
// Initializes LoRa library.
#include "MessageHandler.h"
MessageHandler *MessagingLibrary = NULL;

// Sends the voltage at the 5v pin.
// Connect jumper wire between 5v and A1 pins.
#define inputPin A1

// Specifications for ADC.
// See setup() for references.
// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f
#define voltageReference 5.0f

long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // present interval between sends

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) wait(5000); // wait for serial port to be ready
  #ifdef DEBUG
    Serial.println("Node is active");
  #endif
  
  // Initialize Messaging and LoRa libraries
  MessagingLibrary = new MessageHandler(localAddress);

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();
  
  // Configure analog digital conversion (ADC).
  // MKR WAN 1310 is a SAMD board.
  // Default reference is 5v.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pin.
  // Input pins will take up to 5vdc.
  // We connect it to the 5v pin using a jumper.
  pinMode(inputPin, INPUT);

  // Ready
  #ifdef DEBUG
    Serial.println("=================================================================");
    Serial.println("Arduino MKR 1310 Grassroots LoRa flood-messaging sensor node test");
    Serial.println("=================================================================\n");
  #endif
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    // Read sensor data
    float adcValue = (float)analogRead(inputPin);
    float volts = (adcValue / analogResolution) * voltageReference;

    // Compose message
    String message = "Volts: " + (String)volts;
    
    // Send text message
    uint8_t destinationAddress = 03;
    MessagingLibrary->SendTextMessage(message, destinationAddress);
    #ifdef DEBUG
      Serial.print("Sent text message: "); Serial.print(message);
      Serial.println(" (Text Length: " + (String)message.length() + ")");
      const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();
      for(uint8_t i = MESSAGE_HEADER_LENGTH;
          i < thisMessage[LOCATION_MESSAGE_LENGTH];
          i++) Serial.print((char)thisMessage[i]);
      Serial.println('\n');
    #endif

    // Select the next time to send
    lastSendTime = millis();
    interval = random(maxInterval);
  }
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 00;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
