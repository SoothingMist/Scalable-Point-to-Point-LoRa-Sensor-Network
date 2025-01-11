/**
 * @file FloodMessaging_relay.ino
 *
 * @brief Derived from ping-pong example by
 * @brief Bernd Giesecke (bernd@giesecke.tk)
 * @brief WisBlock-P2P-RX-TX.ino
 * @brief https://github.com/beegee-tokyo/SX126x-Arduino/tree/master/examples/WisBlock-P2P-RX-TX
 * @brief A baseline LoRa P2P RX/TX example
 */

// Adds ability to interpret various message types.
// Contains DEBUG variable. Includes Arduino.h.
#include "MessageHandling.h"
MessageHandling MessageHandler;

// For working with the transceiver
#include <SX126x-Arduino.h> // Click here to get the library: http://librarymanager/All#SX126x

// Function declarations
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);

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
#define RX_TIMEOUT_VALUE 0
#define TX_TIMEOUT_VALUE 5000

/** Structure for radio event callbacks */
static RadioEvents_t RadioEvents;

/** Time CAD was active */
time_t cad_time;

/** Flag to send a packet */
volatile bool send_now = false;

/**
 * @brief Arduino setup function, called once
 *
 */
void setup(void)
{
	// Prepare LED's BLUE ==> TX, GREEN ==> Received a packet
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, LOW);

	// Initialize Serial for debug output
	Serial.begin(115200);
	time_t serial_timeout = millis();
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
	digitalWrite(LED_GREEN, LOW);

	Serial.println("=====================================");
	Serial.println("RAK11310 SX126x P2P RX/TX test");
	Serial.println("=====================================");

	// Initialize the LoRa chip
	Serial.println("Starting lora_hardware_init");
	lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel
	Radio.SetChannel(RF_FREQUENCY);

	// Set Radio TX configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

	// Set Radio RX configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	digitalWrite(LED_GREEN, HIGH);
	Radio.Rx(RX_TIMEOUT_VALUE);
	Serial.printf("Starting P2P Relay Test\n");
}

/**
 * @brief Arduino loop, runs forever
 *
 */
void loop(void)
{
	if (send_now)
	{
	  digitalWrite(LED_GREEN, LOW);
		digitalWrite(LED_BLUE, HIGH);
		send_now = false;

		//-----------------------------//
		// Send with channel activity detection
		//-----------------------------//
		Radio.Sleep();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cad_time = millis();
		Radio.StartCad();
	}
}

/**
 * @brief Callback after TX finished
 *
 */
void OnTxDone(void)
{
	Serial.println("RAK11310 OnTxDone");
	Radio.Rx(RX_TIMEOUT_VALUE);
	digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);
}

/**
 * @brief Callback after a packet was received
 *
 * @param payload pointer to received payload
 * @param size  size of payload
 * @param rssi RSSI of received packet
 * @param snr SNR of received packet
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("\nRAK11310 OnRxDone");
	memcpy(MESSAGE, payload, size);
	Serial.printf("RAK11310 Received Message\nRSSI=%ddBm, SNR=%d, Size=%d\n", rssi, snr, size);
	Radio.Rx(RX_TIMEOUT_VALUE);
  if(MessageHandler.CheckForRebroadcast(size)) send_now = true;
}

/**
 * @brief Callback on TX timeout (should never happen)
 *
 */
void OnTxTimeout(void)
{
	Serial.println("RAK11310 OnTxTimeout");
	Radio.Rx(RX_TIMEOUT_VALUE);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief Callback on RX timeout (will not happen, we enabled permanent RX)
 *
 */
void OnRxTimeout(void)
{
	Serial.println("RAK11310 OnRxTimeout");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**
 * @brief Callback on RX error, can be CRC mismatch or incomplete transmission
 *
 */
void OnRxError(void)
{
	Serial.println("RAK11310 OnRxError");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

/**
 * @brief Callback after Channel Activity Detection is finished
 *
 * @param cad_result
 * true ==> channel is in use
 * false ==> channel is available
 */
void OnCadDone(bool cad_result)
{
	time_t duration = millis() - cad_time;
	if (cad_result)
	{
		Serial.printf("RAK11310 CAD returned channel busy after %ldms --> Skip sending\n", duration);
	  Radio.Rx(RX_TIMEOUT_VALUE);
	}
	else
	{
		Serial.printf("RAK11310 Channel available after %ldms --> send now\n", duration);
		Radio.Send(MESSAGE, MESSAGE[LOCATION_MESSAGE_INDEX] + 1);
	}
}
