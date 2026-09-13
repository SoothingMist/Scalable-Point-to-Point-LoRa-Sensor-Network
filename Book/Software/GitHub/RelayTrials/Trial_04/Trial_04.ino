/*
   Name：Trial_04
   Description: Demonstration of two relay pairs.
                A relay on each pair alternates at the same time.
                A digital multimeter connected as in the documentation
                goes through a +5vdc/0vdc/-5vdc cycle.
   Type: DZS 5V 2-Channel Relay (https://amzn.to/4l75DsJ)
   Drawn from example in https://www.youtube.com/watch?v=d9evR-K6FAY
*/

// For viewing USB dataflow on serial monitor.
#define DEBUG

// Digital pins that issue relay-control pulses.
// Relay is activated when when pin is HIGH.
// Relay is deactivated when pin is LOW.
// Relay can be normally-on or nomally-off, depending on how it is wired.
// Notation: RelayControl_<which pair>_<INx><blank><which MKR digital pin>
#define RelayControl_1_1 1
#define RelayControl_1_2 2
#define RelayControl_2_1 3
#define RelayControl_2_2 4

void setup()
{
  #ifdef DEBUG
    // Initialize serial port
    Serial.begin(9600);
    while (!Serial) Wait(1000); // wait for serial port to be ready
    Serial.println("Device is active");
  #else
    Wait(1000); // give time for device to settle
    pinMode(0, INPUT); // disable serial comm digital pins
    pinMode(1, INPUT);
  #endif

  // Enable relay-control digital pins.
  digitalWrite(RelayControl_1_1, LOW);
  digitalWrite(RelayControl_1_2, LOW);
  digitalWrite(RelayControl_2_1, LOW);
  digitalWrite(RelayControl_2_2, LOW);
  pinMode(RelayControl_1_1, OUTPUT); 
  pinMode(RelayControl_1_2, OUTPUT); 
  pinMode(RelayControl_2_1, OUTPUT); 
  pinMode(RelayControl_2_2, OUTPUT); 

  #ifdef DEBUG
    // Ready
    Serial.println("Initialization Completed");
  #endif
}

void loop()
{
  // Relays' LEDs blink on and off.
  // Clicks are heard from the relays.
  // Relays' activation alternates.
  // Voltage varies as appropriate.

  // Control of Relay Pair #1 Relay #1.
  // Control of Relay Pair #2 Relay #1.

  digitalWrite(RelayControl_1_1, HIGH);
  digitalWrite(RelayControl_2_1, HIGH);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #1 LED should be ON");
    Serial.println("Relay Pair #2 Relay #1 LED should be ON");
    Serial.println("Voltage should read around +4.5vdc\n");
  #endif
  Wait(5000);
  
  digitalWrite(RelayControl_1_1, LOW);
  digitalWrite(RelayControl_2_1, LOW);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #1 LED should be OFF");
    Serial.println("Relay Pair #2 Relay #1 LED should be OFF");
    Serial.println("Voltage should read 0vdc\n");
  #endif
  Wait(5000);

  // Control of Relay Pair #1 Relay #2.
  // Control of Relay Pair #2 Relay #2.

  digitalWrite(RelayControl_1_2, HIGH);
  digitalWrite(RelayControl_2_2, HIGH);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #2 LED should be ON");
    Serial.println("Relay Pair #2 Relay #2 LED should be ON");
    Serial.println("Voltage should read around -4.5vdc\n");
  #endif
  Wait(5000);
  
  digitalWrite(RelayControl_1_2, LOW);
  digitalWrite(RelayControl_2_2, LOW);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #2 LED should be OFF");
    Serial.println("Relay Pair #2 Relay #2 LED should be OFF");
    Serial.println("Voltage should read 0vdc\n");
  #endif
  Wait(5000);
}

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void Wait(long milliseconds)
{
  long beginTime = millis();
  uint8_t doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}
