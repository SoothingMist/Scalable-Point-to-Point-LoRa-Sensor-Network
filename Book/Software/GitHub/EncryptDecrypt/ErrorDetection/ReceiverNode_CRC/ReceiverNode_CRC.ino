
// Basic LoRa Receiver Node.
// Designed for Arduino MKR WAN 1310 microcontroller with LoRa transceiver.
// Original source for LoRa-sender code:
// https://docs.arduino.cc/tutorials/mkr-wan-1310/lora-send-and-receive

// Bring in the CRC library
#include "crc-16-dnp.h"

// Required for LoRa
#include <LoRa.h>

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
    time_t beginTime = millis(); // delay() is blocking so we do not use that
    while ((millis() - beginTime) < 5000);
    exit(1);
  }

  // Ready
  Serial.println("================================================");
  Serial.println("Arduino MKR 1310 basic LoRa receiver test of CRC");
  Serial.println("================================================");
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet. ");

    // read message
    char message[100];
    uint8_t messageLength = LoRa.read();
    uint8_t crc_highbyte = LoRa.read();
    uint8_t crc_lowbyte = LoRa.read();
    uint16_t crc_code = crc_highbyte;
    crc_code = (crc_code << 8) | crc_lowbyte;
    for(uint8_t c = 0; c < messageLength; c++)
      message[c] = (char)LoRa.read();

    // Check message via CRC
    uint16_t crc_receiver = crcr16dnp((uint8_t*)message, messageLength, 0);
    if(crc_receiver == crc_code)
      Serial.println("CRC Checked Out");
    else
      Serial.println("*** CRC Failed)");
  }
}
