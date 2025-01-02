/**
  * @file send_humidity_temperature_battery.ino
  * @brief Transmit node for LoRa point to point communication.
  * @brief Sends readings: battery, temperature, humidity.
  * @brief Deployed via RAK19001, RAK11310, RAK1901.
  * @brief See documentation for details:
  * @brief https://github.com/SoothingMist/Scalable-Point-to-Point-LoRa-Sensor-Network.
 */

// Packs messages.
// Contains DEBUB variable.
#include "MessageConstruction.h"
MessageConstruction MessageConstructor;

// For working with the transceiver
#include "LoRaWan-Arduino.h" // Click here to get the library: http://librarymanager/All#SX126x
#include "mbed.h"
#include "rtos.h"
rtos::Semaphore cadInProgress;
bool channelBusy = true;
#ifdef DEBUG
  unsigned long int cadTime = 0;
#endif

// For working with the RAK1901 (SHTC3) temperature/humidity sensor
#include <Wire.h>
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
SHTC3 g_shtc3; // Declare an instance of the SHTC3 class (required for temperature/humidity sensor RAK1901)
float percentRH;
float degreesC;

// For working with the battery.
// Designed to read the LIPO battery.
// Be sure the onboard switch is set to the LIPO position.
#define PIN_VBAT WB_A0
#define VBAT_MV_PER_LSB (0.806F)    // 3.0V ADC range and 12 - bit ADC resolution = 3300mV / 4096
#define VBAT_DIVIDER (0.6F)         // 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M))
#define VBAT_DIVIDER_COMP (1.846F)  // Compensation factor for the VBAT divider
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
uint32_t vbat_pin = PIN_VBAT;
float volts;

// Forward function declarations
void OnTxDone(void);
void OnTxTimeout(void);
void send();
void OnCadDone(bool cadResult);
void shtc3_decode_errors(SHTC3_Status_TypeDef message);
void shtc3_read_data(void);
float readVBAT(void);

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
#define TX_TIMEOUT_VALUE 5000

// LoRa chip variables
static RadioEvents_t RadioEvents;

void setup()
{
	// Initialize Serial port
  unsigned long startTime = millis();
	Serial.begin(9600);
  while (!Serial) 
  {
    if ((millis() - startTime) < TX_TIMEOUT_VALUE) delay(100);
    else break;
  }

  #ifdef DEBUG
    Serial.println("=================================================================");
    Serial.println("Initializing LoRaP2P Tx for sending sensor data via text messages");
    Serial.println("=================================================================");
    Serial.println("Serial port initialized");
  #endif
  
  // Initialize LoRa chip.
  lora_rak11300_init();

	// Initialize the Radio callbacks
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = NULL;
  RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = NULL;
	RadioEvents.RxError = NULL;
	RadioEvents.CadDone = OnCadDone;

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
    Serial.println("shtc3 initialized");
    Serial.print("Beginning sensor. Result = "); // Most SHTC3 functions return a variable of the type "SHTC3_Status_TypeDef" to indicate the status of their execution
    shtc3_decode_errors(error_message);          // To start the sensor you must call "begin()", the default settings use Wire (default Arduino I2C port)
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

  // The sensor is listed to work up to 1 MHz I2C speed,
  // but the I2C clock speed is global for all sensors on that bus,
  // so using 400kHz or 100kHz is recommended
  Wire.setClock(400000);

  //=============== Initialize Battery Sensor ======================
  
  // Set the analog-to-digital conversion (ADC) resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  // Let the ADC settle
  delay(10);
  // Get a single ADC battery sample and throw it away
  readVBAT();

  //===========================================

  // Ready to go
  #ifdef DEBUG
    Serial.println("=================================================================");
    Serial.println("Sensor Node: Initialization completed");
    Serial.println("=================================================================");
  #endif
}

void loop()
{
  #ifdef DEBUG
    Serial.println();
  #endif

  // Read the temperature/humidity sensors
	shtc3_read_data();

  // Get the battery voltage
  volts = readVBAT() / 1000.0f;

  // Send a message
  send();

  // Wait a bit and repeat.
  // delay() is blocking and there is a lot going on in the background.
  unsigned long startTime = millis();
  while(millis() - startTime < TX_TIMEOUT_VALUE);
}

/** @brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void)
{
  #ifdef DEBUG
	  Serial.println("OnTxDone");
  #endif
}

/**@brief Function to be executed on Radio Tx Timeout event
 */
void OnTxTimeout(void)
{
  #ifdef DEBUG
	  Serial.println("OnTxTimeout");
  #endif
}

/** @brief Function to transmit a message
 */
void send()
{
  char thisText[MAX_MESSAGE_INDEX - MESSAGE_HEADER_LENGTH  + 1]; // notice length limit
  sprintf(thisText, "Batt_V %7.2f, Temp_C %7.2f, Humid_%% %7.2f |",
          volts, degreesC, percentRH);
  MessageConstructor.ComposeMessage_003(2, thisText);
  #ifdef DEBUG
    Serial.print("Message: "); Serial.printf("%s\nHeader:", thisText);
    for(uint8_t i = 0; i < MESSAGE_HEADER_LENGTH; i++) Serial.printf("%4d", (int)MESSAGE[i]);
    Serial.println();
    Serial.printf("Sending message of length %d\n", MESSAGE[LOCATION_MESSAGE_INDEX] + 1);
  #endif

  // CAD to ensure channel not active. Wait until channel not busy.
  // https://news.rakwireless.com/channel-activity-detection-ensuring-your-lora-r-packets-are-sent
  // https://forum.rakwireless.com/t/how-to-calculate-the-best-cad-settings/13984/5
  channelBusy = true; // assume channel is busy
  while(channelBusy) // wait for channel to be not busy before sending a message
  {
    #ifdef DEBUG
      cadTime = millis(); // mark the time CAD process starts
    #endif
    cadInProgress.release(); // reset the in-progress flag
    Radio.Standby(); // put radio on standby
    Radio.SetCadParams(LORA_CAD_08_SYMBOL, LORA_SPREADING_FACTOR + 13, 10, LORA_CAD_RX, 0);
    unsigned long startTime = millis();
    unsigned long stopTime = random(300);
    while(millis() - startTime < stopTime);
    while(!cadInProgress.try_acquire()); // aquire CAD semaphore

    // Start CAD thread
    // (It does seem CAD process runs as an independent thread)
    #ifdef DEBUG
      Serial.println("Starting CAD");
    #endif
    Radio.StartCad();

    // Wait unti OnCadDone finishes
    while(!cadInProgress.try_acquire());
    if(channelBusy)
    {
      #ifdef DEBUG
        Serial.println("Channel Busy");
      #endif
    }
    #ifdef DEBUG
      else Serial.println("Channel not busy");
    #endif
  }

	Radio.Send(MESSAGE, MESSAGE[LOCATION_MESSAGE_INDEX] + 1);
}

/**
   @brief CadDone callback: is the channel busy?
*/
void OnCadDone(bool cadResult)
{
  #ifdef DEBUG
    time_t duration = millis() - cadTime;
    if(cadResult) // true = busy / Channel Activity Detected; false = not busy / channel activity not detected
    {
      Serial.printf("CAD returned channel busy after %ldms\n", duration);
    }
    else
    {
      Serial.printf("CAD returned channel free after %ldms\n", duration);
    }
  #endif

  channelBusy = cadResult;

  #ifdef DEBUG
    osStatus status = cadInProgress.release();
    switch(status)
    {
      case osOK:
        Serial.println("token has been correctly released");
        break;
      case osErrorResource:
        Serial.println("maximum token count has been reached");
        break;
      case osErrorParameter:
        Serial.println("semaphore internal error");
        break;
      default:
        Serial.println("unrecognized semaphore error");
        break;
    }
  #endif
  #ifndef DEBUG
    cadInProgress.release();
  #endif
}

/**
 * @brief Decode messages from the temperature/humidity sensor.
 * @brief Prints "SHTC3_Status_TypeDef" results in a human-friendly way.
 * @param message
 *    Raw message to be decoded
 */
void shtc3_decode_errors(SHTC3_Status_TypeDef message)
{
  #ifdef DEBUG
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
  #endif
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
      shtc3_decode_errors(g_shtc3.lastStatus);
      Serial.println();
    #endif
    percentRH = NAN; // reading is invalid
 		degreesC = NAN;

  }
}

/**
 * @brief Get RAW Battery Voltage
 */
float readVBAT(void)
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
    Serial.printf("The ADC value is: %d\r\n", average_value); // ADC = Analog to Digital Conversion
  #endif

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3300mV and resolution is 12-bit (0..4095)
  float volt = average_value * REAL_VBAT_MV_PER_LSB;
 
  #ifdef DEBUG
    Serial.printf("The battery voltage is: %3.2f mV\r\n", volt);
  #endif
  return volt;
}
