/**
 * @file LoRaP2P_RX_modified.ino
 * @brief modified from RAKwireless' LoRaP2P_RX.ino
 * @brief Receiver node for LoRa point to point communication
 */

// General Arduino IDE capability
#include <Arduino.h>

// For working with the transceiver
#include "LoRaWan-Arduino.h" // Click here to get the library: http://librarymanager/All#SX126x
#include <SPI.h>

// Standard I/O C++ functions
#include <stdio.h>

// General operating environment
#include "mbed.h"
#include "rtos.h"

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
static uint8_t RcvBuffer[64]; // can be no larger than 256

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
            
	// Start LoRa
	Serial.println("Starting Radio.Rx");
	Radio.Rx(RX_TIMEOUT_VALUE);

  // Ready to go
	Serial.println("=====================================");
	Serial.println("LoRaP2P Rx Test");
	Serial.println("=====================================");
}

void loop()
{
 // Put your application tasks here, like reading of sensors,
 // Controlling actuators and/or other functions. 
}

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("OnRxDone");
	delay(10);
	memcpy(RcvBuffer, payload, size);

	Serial.printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);

	for (int idx = 0; idx < size; idx++)
	{
		Serial.printf("%02X ", RcvBuffer[idx]);
	}
  Serial.printf(" : ");
  for (int idx = 0; idx < size; idx++)
  {
    Serial.printf("%c ", RcvBuffer[idx]);
  }
	Serial.println("");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
  static int timeoutCounter = 0;
	Serial.print("OnRxTimeout "); Serial.println(timeoutCounter++);
  //if(timeoutCounter == 5) { timeoutCounter = 0; setup(); } // reset the software
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
	Serial.println("OnRxError");
	Radio.Rx(RX_TIMEOUT_VALUE);
}
