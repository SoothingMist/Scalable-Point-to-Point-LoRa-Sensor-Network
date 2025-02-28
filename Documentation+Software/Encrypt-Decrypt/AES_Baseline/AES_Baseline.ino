
// Ref: https://github.com/SergeyBel/AES
// This is an example of using his library
#include "AES.h"

void setup()
{
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) delay(100);

  // Ready to go
  Serial.println("=========================================");
  Serial.println("Basic AES encrypt/decrypt test");
  Serial.println("=========================================");
}

void loop()
{
  Serial.println("\nStarting Next Cycle");
  
  //plaintext example
  char message[] = "16-byte message:16-byte message\0";
  unsigned char* plain = (unsigned char*)message;
  Serial.print("MsgLen: "); Serial.println(strlen(message) + 1);
  Serial.print("Original:  "); Serial.println((char*)plain);
  //key example
  unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

  // number of bits in plain
  //unsigned int plainLen = 16 * sizeof(unsigned char);
  unsigned int plainLen = (strlen(message) + 1) * sizeof(unsigned char);
  
  // set key
  AES aes(AESKeyLength::AES_128);  ////128 - key length, can be 128, 192 or 256
  
  // encrypt
  unsigned char* encryption = aes.EncryptECB(plain, plainLen, key);
  //Serial.print("Encrypted: "); Serial.println((char*)encryption);

  // decrypt
  unsigned char* decryption = aes.DecryptECB(encryption, plainLen, key); // decrypted message
  Serial.print("Decrypted: "); Serial.println((char*)decryption);

  // Wait a bit. Then repeat the cycle
  delay(2000);
}
