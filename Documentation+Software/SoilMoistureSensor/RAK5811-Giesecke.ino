/**
 * @file RAK5811_0-5V.ino
 * @brief 0 to 5V, A0 and A1 analog input example.
 * @brief Derived from RAKwireless example by Bernd Giesecke.
 * @brief https://github.com/RAKWireless/WisBlock/issues/80
 */
#include <Arduino.h>
#define NO_OF_SAMPLES 32 // Number of samples for average reading
#define ADC0 WB_IO4 // A0 of RAK5811
#define ADC1 WB_A1  // A1 of RAK5811
//#define Calibration mcu_ain_voltage / 0.6; // original
#define Calibration (6.9387f * mcu_ain_voltage) - 0.0560f // calculated for known voltage inputs

void setup()
{
	// Initialize serial port
	time_t timeout = millis();
	Serial.begin(115200);
	while (!Serial)
	{
		if ((millis() - timeout) < 5000)
		{
			delay(100);
		}
		else
		{
			break;
		}
	}

	// Initialize ADC, analog digital converter
	adc_init();
	adc_gpio_init(WB_IO4); // A0 of RAK5811
	adc_gpio_init(WB_A1);  // A1 of RAK5811

	/* WisBLOCK 5811 Power On*/
	pinMode(WB_IO1, OUTPUT);
	digitalWrite(WB_IO1, HIGH);

	/* WisBLOCK 5811 Power On*/
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	// Initialize sampling parameters
	pinMode(ADC0, INPUT_PULLDOWN);
	pinMode(ADC1, INPUT_PULLDOWN);
	analogWriteResolution(12);
}

void loop()
{
	int i;
	float mcu_ain_raw; // raw readubg from analog input
	float average_raw; // average of the raw readings
	float mcu_ain_voltage; // conversion of raw readings to voltage
	float voltage_sensor; // variable calibrated to give the voltage coming from the sensor

	// Reading from A0
	mcu_ain_raw = 0.0f;
	for (i = 0; i < NO_OF_SAMPLES; i++)
	{
		mcu_ain_raw += (float)analogRead(ADC0);				// the input pin A1 for the potentiometer
	}
	average_raw = mcu_ain_raw / (float)NO_OF_SAMPLES;

	mcu_ain_voltage = average_raw * 3.3f / 4095.0f; // ref 3.3V / 12bit ADC

	voltage_sensor = Calibration;

	Serial.printf("mcu_ain_voltage = %f\tA0 = %5.1f\n", mcu_ain_voltage, voltage_sensor);

	// Reading from A1
	mcu_ain_raw = 0.0f;
	for (i = 0; i < NO_OF_SAMPLES; i++)
	{
		mcu_ain_raw += (float)analogRead(ADC1); // the input pin A1 for the potentiometer
	}
	average_raw = mcu_ain_raw / (float)NO_OF_SAMPLES;

	mcu_ain_voltage = average_raw * 3.3f / 4095.0f; // ref 3.3V / 12bit ADC

	voltage_sensor = Calibration;

	Serial.printf("mcu_ain_voltage = %f\tA1 = %5.1f\n", mcu_ain_voltage, voltage_sensor);

	delay(1000);
}
