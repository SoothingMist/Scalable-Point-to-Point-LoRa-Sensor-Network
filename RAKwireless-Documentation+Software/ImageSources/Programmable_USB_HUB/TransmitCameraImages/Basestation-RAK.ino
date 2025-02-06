/**
 * @file LoRaP2P_RX_modified.ino
 * @brief modified from RAKwireless' LoRaP2P_RX.ino
 * @brief Receiver node for LoRa point to point communication
 */

// For use when connected to serial monitor.
// Produces debugging output.
//#define DEBUG

// Constants and variables regarding messages
const uint8_t HANDSHAKE = 110; // ascii code
const uint16_t MAX_MESSAGE_LENGTH = 256;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH]; // first byte is always message length

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

// For working with the transceiver
#include "LoRaWan-Arduino.h" // Click here to get the library: http://librarymanager/All#SX126x
//#include <SPI.h>

// LoRa chip variables
static RadioEvents_t RadioEvents;

// Forward function declarations
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void OnRxError(void);
void Connect();
void ForwardMessage();

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
	Radio.Rx(RX_TIMEOUT_VALUE);

  #ifdef DEBUG
    Serial.println("=====================================");
    Serial.println("Basestation Ready");
    Serial.println("=====================================");
  #else
    Connect();
  #endif
}

void loop()
{
}

// ============= Function Definitions =================

/**@brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  if(size <= MAX_MESSAGE_LENGTH)
  {
    memcpy(MESSAGE, payload, size);

    #ifdef DEBUG
    	Serial.printf("RxDone RSSI=%d dBm, SNR=%d, Length=%d,%d\n", rssi, snr, , size);
    #else
      ForwardMessage();
    #endif
  }
  else
  {
    #ifdef DEBUG
      Serial.printf("RxDone *** RSSI=%d dBm, SNR=%d, Length=%d\n", rssi, snr, size);
    #endif

    OnRxError();
  }
  
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
 */
void OnRxTimeout(void)
{
  #ifdef DEBUG
    static unsigned int timeoutCounter = 0;
	  Serial.print("OnRxTimeout "); Serial.println(++timeoutCounter);
  #endif

	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Error event
 */
void OnRxError(void)
{
  #ifdef DEBUG
    static unsigned int errorCounter = 0;
	  Serial.print("OnRxError "); Serial.println(++errorCounter);
  #endif

	Radio.Rx(RX_TIMEOUT_VALUE);
}

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 1;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 0;
  MESSAGE[1] = 0;
  while((MESSAGE[0] != 1) && (MESSAGE[1] != HANDSHAKE))
  {
    delay(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 1; i < MESSAGE[0] + 1; i++)
  {
    while( ! Serial.available()) delay(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 0; i < MESSAGE[0] + 1; i++) Serial.write(MESSAGE[i]);
}
