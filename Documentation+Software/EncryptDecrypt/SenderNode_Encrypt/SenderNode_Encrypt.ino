
// Basic LoRa Sender Node. Encrypts Messages.
// Designed for Arduino MKR WAN 1310 microcontroller with LoRa transceiver.
// Original source for LoRa-sender code:
// https://docs.arduino.cc/tutorials/mkr-wan-1310/lora-send-and-receive
// Reats and broadcasts the voltage at a given input pin.

// Required for LoRa.
// Includes Arduino.h
#include <LoRa.h>

// Transceiver configuration.
// LoRa packets are composed of overhead and payload (envelope and contents).
// Most commonly, the packet is 256 bytes in length.
// https://www.sciencedirect.com/topics/computer-science/maximum-packet-size
// Using this calculator, https://avbentem.github.io/airtime-calculator/ttn/us915/222,
// we see that message length has to be limited to ensure compliance with maximum time for each transmission.
// Spreading factor and signal bandwidth are set accordingly.
// These factors are for a maximum message (payload) length of 222 bytes.
#define FREQUENCY 915E6
#define SPREADING_FACTOR 7
#define SIGNAL_BANDWIDTH 125E3

// The resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// The voltage reference is 3.3v.
// https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
#define voltageReference 3.3

// Identify the input pin
#define inputPin A1

// Access the encryption library
// https://github.com/SergeyBel/AES
#include "AES.h"
AES aes(AESKeyLength::AES_128); // set key length, can be 128, 192 or 256
unsigned char key[] =
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) // wait for serial port to be ready
  {
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
  }
  Serial.println("Microprocessor is active");

  // Initialize LoRa transceiver.
  // Three options: (433E6, 868E6, 915E6).
  // https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  // National Frequencies:
  // https://www.thethingsnetwork.org/docs/lorawan/frequencies-by-country
  if (!LoRa.begin(915E6))
  {
    Serial.println("Starting LoRa failed!");
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
    exit(1);
  }
  Serial.println("Transceiver is active");
  // From calculator using payload size of 222 and overhead of 13.
  // https://avbentem.github.io/airtime-calculator/ttn/us915/222
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.enableCrc(); // rejects corrupted messages without notice
  Serial.print("Frequency: "); Serial.println(FREQUENCY);
  Serial.print("Spreading Factor: "); Serial.println(SPREADING_FACTOR);
  Serial.print("Signal Bandwidth: "); Serial.println(SIGNAL_BANDWIDTH);

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

  // Ready
  Serial.println("==================================================");
  Serial.println("Arduino MKR 1310 basic LoRa sender encryption test");
  Serial.println("==================================================");
}

void loop()
{
  // Counts the number of packets sent.
  static int counter = 0;

  // Read the input pin
  float adcValue = (float)analogRead(inputPin);
  float volts = (adcValue / analogResolution) * voltageReference;

  // Begin transmit process
  counter++;
  Serial.print("\nSending LoRa P2P message # ");
  Serial.println(counter);

  // Compose message. Do not exceed messageLength;
  const uint8_t messageLength = 50;
  char message[messageLength];
  for(uint8_t c = 0; c < messageLength; c++) message[c] = ' ';
  sprintf(message, "Packet %5d : Value %5.1f", counter, volts);
  int actualLength = strlen(message);
  Serial.print(message);
  Serial.println("(Length " + String(actualLength) + ")");
  message[actualLength] = ' ';
  actualLength += (16 - (actualLength % 16));
  message[actualLength] = '\0';
  Serial.print(message);
  Serial.println("(Length " + String(actualLength) + ")");

  // Encrypt the message
  unsigned int numMessageBits = actualLength * sizeof(unsigned char);
  unsigned char* encryption = aes.EncryptECB((unsigned char*)message, numMessageBits, key);

  // Send packet
  LoRa.beginPacket();
  LoRa.write((unsigned char)actualLength);
  LoRa.write(encryption, actualLength);
  LoRa.endPacket();
  delete [] encryption; // have to delete this after no longer needed to prevent memory leak

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 10000);
}
