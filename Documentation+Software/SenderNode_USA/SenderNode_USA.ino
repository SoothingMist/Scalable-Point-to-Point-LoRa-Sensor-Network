
// Basic LoRa Sender Node.
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
  Serial.println("=============================================");
  Serial.println("Arduino MKR 1310 basic LoRa sender test");
  Serial.println("=============================================");
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
  Serial.print("\nSending packet: ");
  Serial.println(counter);

  // Compose message packet
  char message[100];
  sprintf(message, "Packet %d : Value %5.1f\0", counter, volts);

  // Send packet
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println(message);

  // Wait a bit before repeating
  time_t beginTime = millis();
  while ((millis() - beginTime) < 1000);
}
