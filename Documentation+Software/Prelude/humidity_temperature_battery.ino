/**
   @brief Derived from RAK11300_Battery_Level_Detect and RAK1901_Temperature_Humidity_SHTC3
   @brief Setup and read values from a SHTC3 temperature and humidity sensor
   @brief Read charging level from a battery connected to the baseboard
**/

// If debugging output is not desired, comment-out this line.
//#define DEBUG

// General Arduino IDE capability
#include <Arduino.h>

// For working with the RAK1901 (SHTC3) temperature/humidity sensor
#include <Wire.h>
#include "SparkFun_SHTC3.h"     // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
SHTC3 g_shtc3;						      // Declare an instance of the SHTC3 class (required for temperature/humidity sensor RAK1901)

// For working with the battery
#define PIN_VBAT WB_A0
#define VBAT_MV_PER_LSB (0.806F)    // 3.0V ADC range and 12 - bit ADC resolution = 3300mV / 4096
#define VBAT_DIVIDER (0.6F)         // 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M))
#define VBAT_DIVIDER_COMP (1.846F)  // Compensation factor for the VBAT divider
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
uint32_t vbat_pin = PIN_VBAT;

/**
 * @brief Decode messages from the temperature/humidity sensor
 * @param message
 *    Raw message to be decoded
 */
void errorDecoder(SHTC3_Status_TypeDef message) // The errorDecoder function prints "SHTC3_Status_TypeDef" results in a human-friendly way
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
    float percentRH = g_shtc3.toPercent();
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
		float degreesC = g_shtc3.toDegC();
    
		if (g_shtc3.passTcrc) 						            // Like "passIDcrc" this is true when the T value is valid from the sensor (but not necessarily up-to-date in terms of time)
		{
			Serial.println(degreesC);
		}
		else
		{
			Serial.println("xx.xx");
		}
	}
	else
	{
    Serial.println("), dC = xx.xx");
    Serial.println("), %RH = xx.xx");

    #ifdef DEBUG
		errorDecoder(g_shtc3.lastStatus);
		Serial.println();
   #endif
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
  Serial.printf("\r\nThe Analog Digital Conversion (ADC) value is: %d\r\n", average_value); // ADC = Analog to Digital Conversion
  #endif
  
  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3300mV and resolution is 12-bit (0..4095)
  float volt = average_value * REAL_VBAT_MV_PER_LSB;
 
  //Serial.printf("The battery voltage is: %3.2f mV\r\n", volt);
  Serial.printf("Battery mV = %3.2f \r\n", volt);
  return volt;
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
    if (mvolts < 3300)
        return 0;

    if (mvolts < 3600)
    {
        mvolts -= 3300;
        return mvolts / 30 * 2.55;
    }

    mvolts -= 3600;
    return (10 + (mvolts * 0.15F)) * 2.55;
}

void setup()
{
  // Initialize serial port
	time_t timeout = millis();
	Serial.begin(115200);
	while (!Serial)
	{
		if ((millis() - timeout) < 5000) delay(100);
    else break;
	}

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
  
  Wire.setClock(400000);                      // The sensor is listed to work up to 1 MHz I2C speed, but the I2C clock speed is global for all sensors on that bus so using 400kHz or 100kHz is recommended
  // Set the analog-to-digital conversion (ADC) resolution to 12-bit (0..4095)
  analogReadResolution(12); // Can be 8, 10, 12 or 14
  // Let the ADC settle
  delay(10);
  // Get a single ADC sample and throw it away
  readVBAT();

//===========================================

}

void loop()
{
  // Read the temperature/humidity sensors
	shtc3_read_data();
  //===========================================
  
  // Read the battery voltage
  
  // Get a raw ADC reading
  float vbat_mv = readVBAT();

  /*
  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);

  // Display the results
  char data[32] = {0};

  Serial.print("Power voltage: ");
  memset(data, 0, sizeof(data));
 
  sprintf(data,  "%.3fV \t %d%%\n", vbat_mv / 1000, vbat_per);
  Serial.printf(data);
   
  Serial.print("LIPO = ");
  Serial.print(vbat_mv);
  Serial.print(" mV (");
  Serial.print(vbat_per);
  Serial.print("%)  Battery ");
  Serial.println(mvToLoRaWanBattVal(vbat_mv));
  */
  //===========================================

    delay(1000);
}
