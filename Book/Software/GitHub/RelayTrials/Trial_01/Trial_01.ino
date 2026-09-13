/*
   Name：Trial_01
   Description: Simple demonstration of one relay
   Type: DZS 5V 2-Channel Relay (https://amzn.to/4l75DsJ)
   Drawn from example in https://www.youtube.com/watch?v=d9evR-K6FAY
*/

// For viewing USB dataflow on serial monitor.
#define DEBUG

// Arduino digital pin that issues relay-control pulse.
// Relay is activated when when pin is HIGH.
// Relay is deactivated when pin is LOW.
// Notice that relay can be normally-on or nomally-off,
// depending on how the relay is wired.
#define RelayControl 2

void setup()
{
  #ifdef DEBUG
    // Initialize serial port
    Serial.begin(9600);
    while (!Serial) Wait(1000); // wait for serial port to be ready
    Serial.println("Device is active");
  #else
    Wait(1000); // give time for device to settle
    pinMode(0, INPUT); // disables serial comm pins
    pinMode(1, INPUT);
  #endif

  // Enable relay-control digital pin
  digitalWrite(RelayControl, LOW);
  pinMode(RelayControl, OUTPUT); 

  #ifdef DEBUG
    // Ready
    Serial.println("Initialization Completed");
  #endif
}

void loop()
{
  // Relay's LED should blink on and off.
  // Clicks should be heard from the relay.
  
  digitalWrite(RelayControl, HIGH);
  #ifdef DEBUG
    Serial.println("LED should be ON");
  #endif
  Wait(5000);
  
  digitalWrite(RelayControl, LOW);
  #ifdef DEBUG
    Serial.println("LED should be OFF");
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
