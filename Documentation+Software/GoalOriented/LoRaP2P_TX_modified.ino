/**
  * @file LoRaP2P_TX_modified.ino
  * @brief modified from RAKwireless' LoRaP2P_TX.ino
  * @brief Transmit node for LoRa point to point communication
 */

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

// Function declarations
void OnTxDone(void);
void OnTxTimeout(void);

// Define LoRa parameters
#define RF_FREQUENCY 915000000	// Hz
#define TX_OUTPUT_POWER 22		  // dBm
#define LORA_BANDWIDTH 0		    // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1		    // [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8	// Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0	  // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000

// LoRa chip variables
static RadioEvents_t RadioEvents;
static uint8_t TxdBuffer[64];

void setup()
{
	// Initialize Serial port
	Serial.begin(9600);
	while (!Serial) delay(100);
 
  // Initialize LoRa chip.
  lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.TxTimeout = OnTxTimeout;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel
	Radio.SetChannel(RF_FREQUENCY);

	// Set Radio TX configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

	Serial.println("=====================================");
	Serial.println("LoRaP2P Tx Test");
	Serial.println("=====================================");
}

void loop()
{
  // Put your application tasks here, like reading of sensors,
  // Controlling actuators and/or other functions.

  // Send test messages
  send();
  delay(1000);
}

/**@brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
	Serial.println("OnTxDone");
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
	Serial.println("OnTxTimeout");
}

void send()
{
	TxdBuffer[0] = 'H';
	TxdBuffer[1] = 'e';
	TxdBuffer[2] = 'l';
	TxdBuffer[3] = 'l';
	TxdBuffer[4] = 'o';
	Radio.Send(TxdBuffer, 5);
}
