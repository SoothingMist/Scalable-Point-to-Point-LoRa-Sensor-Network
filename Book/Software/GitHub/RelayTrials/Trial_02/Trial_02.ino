/*
   Name：Trial_02
   Description: Simple demonstration of both relays in a relay pair
   Type: DZS 5V 2-Channel Relay (https://amzn.to/4l75DsJ)
   Drawn from example in https://www.youtube.com/watch?v=d9evR-K6FAY
*/

// For viewing USB dataflow on serial monitor.
#define DEBUG

// Digital pins that issue relay-control pulses.
// Relay is activated when when pin is HIGH.
// Relay is deactivated when pin is LOW.
// Notice that relay can be normally-on or nomally-off,
// depending on how it is wired.
#define RelayControl_1 1
#define RelayControl_2 2

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
  digitalWrite(RelayControl_1, LOW);
  digitalWrite(RelayControl_2, LOW);
  pinMode(RelayControl_1, OUTPUT); 
  pinMode(RelayControl_2, OUTPUT); 

  #ifdef DEBUG
    // Ready
    Serial.println("Initialization Completed");
  #endif
}

void loop()
{
  // Relays' LED blink on and off.
  // Clicks are heard from the relays.
  // Relays' activation alternates.

  // Control of Relay #2.

  digitalWrite(RelayControl_2, HIGH);
  #ifdef DEBUG
    Serial.println("LED #2 should be ON");
  #endif
  Wait(5000);
  
  digitalWrite(RelayControl_2, LOW);
  #ifdef DEBUG
    Serial.println("LED #2 should be OFF");
  #endif
  Wait(5000);

  // Control of Relay #1.

  digitalWrite(RelayControl_1, HIGH);
  #ifdef DEBUG
    Serial.println("LED #1 should be ON");
  #endif
  Wait(5000);
  
  digitalWrite(RelayControl_1, LOW);
  #ifdef DEBUG
    Serial.println("LED #1 should be OFF");
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
