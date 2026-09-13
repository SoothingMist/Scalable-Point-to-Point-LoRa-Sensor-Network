
// Grassroots Sensor Node.
// Periodically broadcasts its sensor value.

// For viewing Serial.println text on serial monitor.
#define DEBUG

// Unique address of this network node
#define localAddress 1

// Library for message handling.
// Includes LoRa library.
#include <LoRaMessageHandler.h>
LoRaMessageHandler *LoRaMessagingLibrary = NULL;

// Sends the voltage at the selected pin.
// Connect jumper wire between GND and A1 pins.
#define inputPin A1

// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// The voltage reference is 3.3v.
// https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
#define voltageReference 3.3f

long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // present interval between sends

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }
  #ifdef DEBUG
    Serial.println("Microprocessor is active");
  #endif
  
  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();
  
  // Configure analog digital conversion (ADC).
  // MKR WAN 1310 is a SAMD board.
  // Default reference is given by AR_DEFAULT.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pin.
  pinMode(inputPin, INPUT);

  // Initialize message-handling library.
  LoRaMessagingLibrary = new LoRaMessageHandler(localAddress);
  
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
    char message[100];
    sprintf(message, "Volts: %5.1f\0", volts);
    #ifdef DEBUG
      Serial.print("Trying to send this message: "); Serial.println(String(message));
    #endif

    // Send text message
    uint8_t destinationAddress = 3;
    LoRaMessagingLibrary->SendTextMessage(message, destinationAddress);
    #ifdef DEBUG
      char* MESSAGE = (char*)LoRaMessagingLibrary->getMESSAGE();
      Serial.print("Sent this message: ");
      for(uint8_t i = MESSAGE_HEADER_LENGTH;
          i < MESSAGE[LOCATION_MESSAGE_LENGTH];
          i++) Serial.print((char)MESSAGE[i]);
      Serial.println(" (Text Length: " + String(String(message).length()) + ")\n");
    #endif

    // Select the next time to send
    lastSendTime = millis();
    interval = random(maxInterval);
  }
}
