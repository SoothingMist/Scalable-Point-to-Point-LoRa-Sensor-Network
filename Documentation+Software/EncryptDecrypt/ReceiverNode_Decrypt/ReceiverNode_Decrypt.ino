
// Basic LoRa Receiver Node. Decrypts messages.
// Designed for Arduino MKR WAN 1310 microcontroller with LoRa transceiver.
// Original source for LoRa-sender code:
// https://docs.arduino.cc/tutorials/mkr-wan-1310/lora-send-and-receive

// Required for LoRa
#include <LoRa.h>

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
  Serial.println("====================================================");
  Serial.println("Arduino MKR 1310 basic LoRa receiver decryption test");
  Serial.println("====================================================");
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet. ");

    // Get length of message
    unsigned char messageLength = LoRa.read();
    unsigned int numMessageBits = (unsigned int)messageLength * sizeof(unsigned char);
    unsigned char encryption[50]; // do not exceed this length;

    // Read message
    for(int c = 0; c < messageLength; c++)
      encryption[c] = LoRa.read();

    // Decrypt and display message
    unsigned char* decryption = aes.DecryptECB(encryption, numMessageBits, key); // decrypted message
    Serial.print(F("Decrypted. "));
    for(int c = 0; c < messageLength; c++) Serial.print((char)decryption[c]); Serial.println();
    delete [] decryption; // have to delete this after no longer needed to prevent memory leak

    // print RSSI of packet
    Serial.print("'  RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
