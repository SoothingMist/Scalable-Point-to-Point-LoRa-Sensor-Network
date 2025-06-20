
#include "crc-16-dnp.h"

void setup()
{
  // put your setup code here, to run once:

  // Configure serial communications
  Serial.begin(9600);
  while(!Serial) delay(100);
  Serial.println("\nDevice is Ready\n"); Serial.flush();
}

void loop()
{
  // Simulate a message
  String message = "";
  long length = random(10, 101);
  for(long c = 0; c < length; c++)
    message += (char)random(256);

  // Calculate CRC for that message  
  uint16_t crc_sender = crcr16dnp((uint8_t*)message.c_str(), length, 0);

  // Simulate "transmission" of message
  char* messageVector = (char*)malloc(length * sizeof(char));
  for(long c = 0; c < length; c++)
    messageVector[c] = message.c_str()[c];

  // Check CRC
  uint16_t crc_receiver = crcr16dnp((uint8_t*)messageVector, length, 0);
  if(crc_receiver == crc_sender)
    Serial.println("CRC Checked Out");
  else
    Serial.println("*** CRC Failed)");

  // Get ready for another round
  free(messageVector);
  delay(1000);
}
