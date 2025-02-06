/**
  * @file send_humidity_temperature_battery.ino
  * @brief Transmit node for LoRa point to point communication.
  * @brief Sends readings: battery, temperature, humidity.
  * @brief Encrypts outgoing messages.
  * @brief Deployed via RAK19001, RAK11310, RAK1901.
  * @brief See documentation for details:
  * @brief https://github.com/SoothingMist/Scalable-Point-to-Point-LoRa-Sensor-Network.
***/

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

// For encryption/decryption
// Ref: https://github.com/SergeyBel/AES
#include "AES.h"
// example key - 16 bytes
unsigned char key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
// set key
AES aes(AESKeyLength::AES_128);  ////128 - key length, can be 128, 192 or 256
// Message length must be evenly divisable by 16 and no larger than 256.
// LoRa messages can be no longer than 256 bytes.
const unsigned int maxMsgLen = 256;
uint8_t TxBuffer[maxMsgLen];

// For working with the RAK1901 (SHTC3) temperature/humidity sensor
#include <Wire.h>
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
SHTC3 g_shtc3;              // Declare an instance of the SHTC3 class (required for temperature/humidity sensor RAK1901)
float percentRH;
float degreesC;

// Function declarations
void OnTxDone(void);
void OnTxTimeout(void);
void send();
void shtc3_decode_errors(SHTC3_Status_TypeDef message);
void shtc3_read_data(void);
void readVBAT(void);
uint8_t mvToPercent(float mvolts);
uint8_t mvToLoRaWanBattVal(float mvolts);

// For working with the battery
#define PIN_VBAT WB_A0
#define VBAT_MV_PER_LSB (0.806F)    // 3.0V ADC range and 12 - bit ADC resolution = 3300mV / 4096
#define VBAT_DIVIDER (0.6F)         // 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M))
#define VBAT_DIVIDER_COMP (1.846F)  // Compensation factor for the VBAT divider
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
uint32_t vbat_pin = PIN_VBAT;
float volt;

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

void setup()
{
	// Initialize Serial port
	Serial.begin(9600);
	while (!Serial) delay(100);
 
  // Initialize LoRa chip.
  lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = NULL;
  RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = NULL;
	RadioEvents.RxError = NULL;
	RadioEvents.CadDone = NULL;

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel (operating frequency)
	Radio.SetChannel(RF_FREQUENCY);

	// Set Radio TX configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

  //=============== Initialize Humidity/Temperature Sensor ======================
  
	Wire.begin();

  #ifndef DEBUG
    g_shtc3.begin();
  #else
    SHTC3_Status_TypeDef error_message = g_shtc3.begin();
  #endif
  
  // Diagnostics
  #ifdef DEBUG
    Serial.println("shtc3 init");
    Serial.print("Beginning sensor. Result = "); // Most SHTC3 functions return a variable of the type "SHTC3_Status_TypeDef" to indicate the status of their execution
    errorDecoder(error_message);                 // To start the sensor you must call "begin()", the default settings use Wire (default Arduino I2C port)
    Serial.println();

    if (g_shtc3.passIDcrc)                      // Whenever data is received the associated checksum is calculated and verified so you can be sure the data is true
    {					   						                    // The checksum pass indicators are: passIDcrc, passRHcrc, and passTcrc for the ID, RH, and T readings respectively
      Serial.print("ID Passed Checksum. ");
      Serial.print("Device ID: 0b");
      Serial.println(g_shtc3.ID, BIN); 		      // The 16-bit device ID can be accessed as a member variable of the object
    }
    else
    {
      Serial.println("ID Checksum Failed. ");
    }
  #endif
  
  //=============== Initialize Battery Sensor ======================
  
  // The sensor is listed to work up to 1 MHz I2C speed,
  // but the I2C clock speed is global for all sensors on that bus,
  // so using 400kHz or 100kHz is recommended
  Wire.setClock(400000);
  // Set the analog-to-digital conversion (ADC) resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  // Let the ADC settle
  delay(10);
  // Get a single ADC battery sample and throw it away
  readVBAT();

  //===========================================

  // Ready to go
	Serial.println("=================================================");
	Serial.println("LoRaP2P Tx test for sending encrypted sensor data");
	Serial.println("=================================================");
}

void loop()
{
  // Put your application tasks here, like reading of sensors,
  // Controlling actuators and/or other functions.

  // Read the temperature/humidity sensors
	shtc3_read_data();

  // Read the battery voltage
  // Get a raw ADC reading
  readVBAT();

  // Send a message
  send();
  delay(2000);
}

/** @brief Function to be executed on Radio Tx Done event
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

/** @brief Function to transmit a message
 */
void send()
{
  // Construct the message
  for(unsigned int c = 0; c < maxMsgLen; c++) TxBuffer[c] = (uint8_t)'\0'; // clear the message buffer
  sprintf((char*)TxBuffer, "Sensors: Battery_Volts %7.2f, Temperature_C %7.2f, Humidity_%% %7.2f",
          volt / 1000.0, degreesC, percentRH);
  
  // Encrypt the message
  unsigned int messageLength = strlen((char*)TxBuffer);
  if(messageLength != ((messageLength / 16) * 16))
    messageLength = (16 * (messageLength / 16)) + 16; // length of padded message
  Serial.print("\nPadded message length: "); Serial.println(messageLength);
  uint8_t* encryption = aes.EncryptECB(TxBuffer, messageLength * sizeof(uint8_t), key);

  // Send the message
	Radio.Send(encryption, messageLength);
  Serial.print("Sent:"); Serial.println((char*)TxBuffer);
}

/**
 * @brief Decode messages from the temperature/humidity sensor.
 * @brief Prints "SHTC3_Status_TypeDef" results in a human-friendly way.
 * @param message
 *    Raw message to be decoded
 */
void shtc3_decode_errors(SHTC3_Status_TypeDef message)
{
  switch (message)
  {
    case SHTC3_Status_Nominal:
      Serial.print("Nominal");
      break;
    case SHTC3_Status_Error:
      Serial.print("Error");
      break;
    case SHTC3_Status_CRC_Fail:
      Serial.print("CRC Fail");
      break;
    default:
      Serial.print("Unknown return code");
      break;
  }
}

/**
 * @brief Get readings from the temperature/humidity sensor
 */
void shtc3_read_data(void)
{
	g_shtc3.update();
 
	if (g_shtc3.lastStatus == SHTC3_Status_Nominal) // You can also assess the status of the last command by checking the ".lastStatus" member of the object
	{
    // Get sensor data values
    percentRH = g_shtc3.toPercent();
 		degreesC = g_shtc3.toDegC();

    #ifdef DEBUG
      Serial.print("%RH = ");
      if (g_shtc3.passRHcrc) 						            // Like "passIDcrc" this is true when the RH value is valid from the sensor (but not necessarily up-to-date in terms of time)
      {
        Serial.println(percentRH);
      }
      else
      {
        Serial.println("xx.xx");
      }
      
      Serial.print("dC = ");
      
      if (g_shtc3.passTcrc) 						            // Like "passIDcrc" this is true when the T value is valid from the sensor (but not necessarily up-to-date in terms of time)
      {
        Serial.println(degreesC);
      }
      else
      {
        Serial.println("xx.xx");
      }
    #endif
  }
  else
  {
    #ifdef DEBUG
      Serial.println("), dC = xx.xx");
      Serial.println("), %RH = xx.xx");
      errorDecoder(g_shtc3.lastStatus);
      Serial.println();
    #endif
    percentRH = NAN; // reading is invalid
 		degreesC = NAN;

  }
}

/**
 * @brief Get RAW Battery Voltage
 */
void readVBAT(void)
{
  unsigned int sum = 0,average_value = 0;
  unsigned int read_temp[10] = {0};
  unsigned char i = 0;
  unsigned int adc_max = 0;
  unsigned int adc_min = 4095; 
  average_value = analogRead(vbat_pin);
  for(i=0;i<10;i++)
  {
    read_temp[i] = analogRead(vbat_pin);
    if(read_temp[i] < adc_min)  
      {
        adc_min = read_temp[i];
      }
    if(read_temp[i] > adc_max)  
      {
        adc_max = read_temp[i];
      }
     sum = sum + read_temp[i];
  }
  average_value = (sum - adc_max - adc_min) >> 3;

  #ifdef DEBUG
    Serial.printf("\r\nThe Analog Digital Conversion (ADC) value is: %d\r\n", average_value); // ADC = Analog to Digital Conversion
  #endif
  
  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3300mV and resolution is 12-bit (0..4095)
  volt = average_value * REAL_VBAT_MV_PER_LSB;
 
  #ifdef DEBUG
    Serial.printf("Battery mV = %3.2f \r\n", volt);
  #endif
}

/**
 * @brief Convert from raw mv to percentage
 * @param mvolts
 *    RAW Battery Voltage
 */
uint8_t mvToPercent(float mvolts)
{
    if (mvolts < 3300)
        return 0;

    if (mvolts < 3600)
    {
        mvolts -= 3300;
        return mvolts / 30;
    }

    mvolts -= 3600;
    return 10 + (mvolts * 0.15F); // thats mvolts /6.66666666
}

/**
 * @brief get LoRaWan Battery value
 * @param mvolts
 *    Raw Battery Voltage
 */
uint8_t mvToLoRaWanBattVal(float mvolts)
{
    if (mvolts < 3300) return 0;

    if (mvolts < 3600)
    {
        mvolts -= 3300;
        return mvolts / 30 * 2.55;
    }

    mvolts -= 3600;
    return (10 + (mvolts * 0.15F)) * 2.55;
}
