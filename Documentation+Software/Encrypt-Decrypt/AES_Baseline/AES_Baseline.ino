
// Add ability to check remaining memory.
// https://docs.arduino.cc/learn/programming/memory-guide
extern "C" char* sbrk(int incr);

// Ref: https://github.com/SergeyBel/AES
// This is an example of using his encryption library
#include "AES.h"
AES aes(AESKeyLength::AES_128); // set key length, can be 128, 192 or 256
unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };


void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) delay(100);
}

void loop()
{
  // Construct a message
  const unsigned int messageLength = 16;
  unsigned int numMessageBits = messageLength * sizeof(unsigned char);
  char message[messageLength];
  for(unsigned int i = 0; i < messageLength; i++) message[i] = ' ';
  sprintf(message, "%d", freeRam());
  Serial.print(F("Message Length: ")); Serial.println(messageLength);
  Serial.print(F("Original:  ")); Serial.println(message);

  // encrypt
  unsigned char* encryption = aes.EncryptECB((unsigned char*)message, numMessageBits, key);
  
  // decrypt
  unsigned char* decryption = aes.DecryptECB(encryption, numMessageBits, key); // decrypted message
  Serial.print(F("Decrypted: ")); Serial.println((char*)decryption);

  // Next cycle
  delete [] encryption; // have to delete this after no longer needed to prevent memory leak
  delete [] decryption; // have to delete this after no longer needed to prevent memory leak
  delay(500);
  Serial.println();
}

// Check remaining memory.
int freeRam()
{
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
