/**
  * @file  FloodMessaging_Basestation_RAKwireless.ino
  * @brief Basestation node for LoRa flood-messaging network.
  * @brief Relays messages within the network.
  * @brief Deployed via RAK19001 and RAK11310.
  * @brief See documentation for details:
  * @brief https://github.com/SoothingMist/Scalable-Point-to-Point-LoRa-Sensor-Network.
 */

// Adds ability to interpret various message types.
// Contains DEBUG variable.
#include "MessageHandling.h"
MessageHandling MessageHandler;

// For working with the transceiver
#include "LoRaWan-Arduino.h" // Click here to get the library: http://librarymanager/All#SX126x

// Forward function declarations
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

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
#define RX_TIMEOUT_VALUE 0
#define TX_TIMEOUT_VALUE 3000

// LoRa chip variables
static RadioEvents_t RadioEvents;

void setup()
{
	// Initialize Serial port
	Serial.begin(115200);
	time_t serial_timeout = millis();
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
		}
		else
		{
			break;
		}
	}

  // Initialize LoRa chip.
  lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = NULL;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = NULL;
	RadioEvents.RxTimeout = NULL;
	RadioEvents.RxError = NULL;
	RadioEvents.CadDone = NULL;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel (operating frequency)
	Radio.SetChannel(RF_FREQUENCY);

	// Set Radio RX configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  // Ready to go
	Radio.Rx(RX_TIMEOUT_VALUE);
  #ifdef DEBUG
    Serial.print("=======================================");
    Serial.print("\nLoRaP2P Basestation Ready\n");
    Serial.print("=======================================");
  #endif
}

void loop()
{
  // There is nothing here. 
  // Callbacks from the transceiver are used instead.
}

void OnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
  #ifdef DEBUG
    Serial.println("\n\nOnRxDone");
    Serial.printf("Size = %d bytes, RssiValue = %d dBm, SnrValue = %d\n", (int)size, (int)rssi, (int)snr);
  #endif

	// Decide whether or not to pass on to the PC component of the basestation.
  memcpy(MESSAGE, payload, size);
  if(MessageHandler.CheckForPassthrough(size))
  {
    #ifdef DEBUG
      Serial.println("Received true. Message passed forward:");
      for (uint16_t i = MESSAGE_HEADER_LENGTH; i < size; i++) Serial.printf("%c", payload[i]);
      Serial.println();
    #endif
    Serial.write(MESSAGE, MESSAGE[LOCATION_MESSAGE_INDEX] + 1);
  }
  #ifdef DEBUG
    else
    {
        Serial.println("Received false. Message not passed forward.");
    }
  #endif

	Radio.Rx(RX_TIMEOUT_VALUE);
}
