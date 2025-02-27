
// Basic LoRa Sender Node.
// Designed for Arduino MKR WAN 1310 microcontroller with LoRa transceiver.
// Original source for LoRa-sender code:
// https://docs.arduino.cc/tutorials/mkr-wan-1310/lora-send-and-receive

// Required for LoRa
#include <SPI.h>
#include <LoRa.h>

// Adds Arduino's language capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
#include <Arduino.h>

// Sends the voltage at the 5v pin.
// Connect jumper wire between 5v and A1 pins.
#define inputPin A1

// Specifications for ADC.
// See setup() for references.
// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f
#define voltageReference 5.0f

// Counts the number of packets sent.
int counter = 0;

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }

  // Initialize LoRa transceiver.
  // Three options: (433E6, 868E6, 915E6).
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(915E6))
  {
    Serial.println("Starting LoRa failed!");
    exit(1);
  }

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
  Serial.println("=============================================");
  Serial.println("Arduino MKR 1310 basic LoRa basic sender test");
  Serial.println("=============================================");
}

void loop()
{
  // Begin transmit process
  counter++;
  Serial.print("\nSending packet: ");
  Serial.println(counter);

  // Read sensor data
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;

  // Compose message packet
  char message[100];
  sprintf(message, "Packet %d : Value %f\0", counter, volts);

  // Send packet
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println(message);

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 1000);
}
