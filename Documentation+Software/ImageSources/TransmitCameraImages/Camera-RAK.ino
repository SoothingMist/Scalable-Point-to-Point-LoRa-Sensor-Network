
// Constants and variables regarding messages
const uint8_t HANDSHAKE = 110; // ascii code
const unsigned int MAX_MESSAGE_LENGTH = 256;
uint8_t MESSAGE[MAX_MESSAGE_LENGTH]; // first byte is always message length
unsigned long cad_time = 0; // time CAD was active
bool tx_finished = true; // false = tx not completed, true = tx completed
bool cad_channel_open = true; // false = channel occupied, true = channel not occupied

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

// For working with the transceiver
#include <SX126x-Arduino.h> // Click here to get the library: http://librarymanager/All#SX126x

// LoRa chip variables
static RadioEvents_t RadioEvents;

// Forward function declarations
void OnTxDone();
void OnCadDone(bool cadResult);
void Connect();
void ForwardMessage();
void ReceiveMessage();
void Broadcast();

void setup()
{
  // Initialize serial port
  // https://docs.arduino.cc/language-reference/en/functions/communication/serial/begin
  // Baudrate = 9600, data bits = 8, parity = none, stop bits = 1
  Serial.begin(9600, SERIAL_8N1);
  while (!Serial) delay(100); // make sure Serial is ready

  //=============== Initialize RAK11310 Transceiver ======================

  // Initialize chip.
  lora_rak11300_init();

	// Initialize radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = NULL;
  RadioEvents.TxTimeout = NULL;
	RadioEvents.RxTimeout = NULL;
	RadioEvents.RxError = NULL;
	RadioEvents.CadDone = OnCadDone;

	// Initialize radio
	Radio.Init(&RadioEvents);

	// Set radio channel (operating frequency)
	Radio.SetChannel(RF_FREQUENCY);

	// Set radio TX configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  //====================================================================

  // Wait for connection with the other device on the USB HUB
  Connect();
  delay(1000);
}

void loop()
{
  // Receive from camera via USB HUB.
  // Each byte can be 0..255.
  ReceiveMessage();

  // Boadcast via LoRaP2P
  Broadcast();
}

// ============= Function Definitions =================

// Handshake with the connected device.
// Reject any input but the handshake.
// Both devices must use the same handshake.
void Connect()
{
  // Send handshake
  MESSAGE[0] = 01;
  MESSAGE[1] = HANDSHAKE;
  ForwardMessage();

  // Receive handshake
  MESSAGE[0] = 00;
  MESSAGE[1] = 00;
  while((MESSAGE[0] != 01) && (MESSAGE[1] != HANDSHAKE))
  {
    delay(100);
    ReceiveMessage();
  }
}

void ReceiveMessage()
{
  while( ! Serial.available()) delay(100); // wait for message
  MESSAGE[0] = (uint8_t)Serial.read(); // get message size
  for(uint8_t i = 01; i <= MESSAGE[0]; i++)
  {
    while( ! Serial.available()) delay(100); // wait for message
    MESSAGE[i] = (uint8_t)Serial.read(); // read message
  }
}

void ForwardMessage()
{
  for(uint8_t i = 00; i <= MESSAGE[0]; i++) Serial.write(MESSAGE[i]);
}

void Broadcast()
{
		//-----------------------------//
		// Send with channel activity detection
		//-----------------------------//
		Radio.Sleep();
		Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_ONLY, 0);
		cad_time = millis();
    tx_finished = false;
    cad_channel_open = false;
		Radio.StartCad();
    while( ! tx_finished) // wait for tx to complete
    {
      unsigned long serial_timeout = millis();
      while((millis() - serial_timeout) < 100);
    }
}

void OnTxDone(void)
{
	//Serial.println("RAK11310 OnTxDone");
  tx_finished = true;
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
	//unsigned long duration = millis() - cad_time;
	if (cad_result)
	{
		//Serial.printf("RAK11310 CAD returned channel busy after %ldms --> Skip sending\n", duration);
	  cad_channel_open = false;
    tx_finished = true;
	}
	else
	{
		Radio.Send(MESSAGE, MESSAGE[0] + 1);
		//Serial.printf("RAK11310 Channel available after %ldms --> send now\n", duration);
    cad_channel_open = true;
	}
}
