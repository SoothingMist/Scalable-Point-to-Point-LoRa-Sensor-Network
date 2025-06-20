
#include "crc-16-dnp.h"

void setup()
{
  // put your setup code here, to run once:

  // Configure serial communications
  Serial.begin(9600);
  while(!Serial) delay(100);
  const size_t MessageSize = 5;
  uint8_t data[MessageSize];
  data[0] = (uint8_t)'1';
  data[1] = (uint8_t)'2';
  data[2] = (uint8_t)'3';
  data[3] = (uint8_t)'\r';
  data[4] = (uint8_t)'\n';
  Serial.write(data, MessageSize);
  for(size_t i = 0; i < MessageSize; i++) Serial.println(data[i]);
  Serial.flush();
  Serial.println(crcr16dnp(data, MessageSize, 0), HEX);
  Serial.println(crcr16dnp(data, MessageSize, 0));
  Serial.println("Finished"); Serial.flush();
}

void loop()
{
  // put your main code here, to run repeatedly.
}
