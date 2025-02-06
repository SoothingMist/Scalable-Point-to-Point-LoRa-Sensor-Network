/**
  * @file LoRaP2P_RX_modified.ino
  * @brief Receiver node for LoRa point to point communication
  * @brief Receives readings: battery, temperature, humidity.
  * @brief Decrypts incoming messages.
  * @brief Deployed via RAK19001, RAK11310.
  * @brief See documentation for details:
  * @brief https://github.com/SoothingMist/Scalable-Point-to-Point-LoRa-Sensor-Network.
***/

// General Arduino IDE capability
#include <Arduino.h>

// For working with the transceiver
#include "LoRaWan-Arduino.h" //http://librarymanager/All#SX126x
#include <SPI.h>

// Standard I/O C++ functions
#include <stdio.h>

// General operating environment
#include "mbed.h"
#include "rtos.h"

// Library for encryption/decryption
// Ref: https://github.com/SergeyBel/AES
#include "AES.h"
// example key - must match that used by sender
unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
// set key - must match that used by sender
AES aes(AESKeyLength::AES_128);
// Message length must be evenly divisable by 16 and no larger than 256.
// (LoRa messages can be no longer than 256 bytes.)
const unsigned int maxMsgLen = 256; // can be no larger than 256
uint8_t RxBuffer[maxMsgLen];

// Function declarations
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void OnRxError(void);

// Define LoRa parameters
#define RF_FREQUENCY 915000000	// Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0	// Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

// LoRa chip variables
static RadioEvents_t RadioEvents;

void setup()
{
	// Initialize Serial port
	Serial.begin(9600);
	while (!Serial) delay(100);

  // Initialize LoRa chip.
  lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = NULL;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = NULL;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = NULL;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel
	Radio.SetChannel(RF_FREQUENCY);

	// Set Radio RX configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, false);
  
  // Ready to go
	Serial.println("========================================================");
	Serial.println("LoRaP2P Rx test for receiving and decrypting sensor data");
	Serial.println("========================================================");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

void loop()
{
  // Put your application tasks here, like reading of sensors,
  // Controlling actuators and/or other functions. 

  // There is nothing here. 
  // Callbacks from the transceiver are used instead.
}

/** @brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  // Receive the message
	Serial.println("\nOnRxDone");
	delay(10);
	memcpy(RxBuffer, payload, size);

  // Report parameters of message
	Serial.printf("Size = %d bytes, RssiValue = %d dBm, SnrValue = %d\n", size, rssi, snr);

  // Decrypt the message
  unsigned char* decryption = aes.DecryptECB(RxBuffer, size, key); // decrypted message

  // Display the message
	Serial.println((char*)decryption);
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/** @brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
  static int timeoutCounter = 0;
	Serial.print("OnRxTimeout "); Serial.println(timeoutCounter++);
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/** @brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError");
	Radio.Rx(RX_TIMEOUT_VALUE);
}
