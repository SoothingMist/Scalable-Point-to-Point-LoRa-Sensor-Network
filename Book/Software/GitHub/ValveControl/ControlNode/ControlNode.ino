/*
   Name：ValveControlNode
   Description: Receives activate/deactivate signals via LoRaP2P.
                Triggers relays appropriately.
                Two relay pairs produce positive and negative pulses
                according to signal received.
   Relay Type: DZS 5V 2-Channel Relay (https://amzn.to/4l75DsJ)
   Drawn from example in https://www.youtube.com/watch?v=d9evR-K6FAY

   Also reads sensors attached to the node.
*/

// For viewing USB dataflow on serial monitor.
//#define DEBUG

// Unique address of this network node.
#define localAddress 3

// Unique address of basestation node.
#define basestationAddress 1

// Library for LoRa message handling.
#include "LoRaMessageHandler-Extended.h"
LoRaMessageHandler *MessagingLibrary = NULL;

// Digital resolution is the number of ADC levels.
// https://electronics.stackexchange.com/questions/406906/how-to-obtain-input-voltage-from-adc-value
#define analogResolution 4096.0f

// The voltage reference is 3.3vdc.
// https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
#define voltageReference 3.3f

// A maximum random wait time between transmissions.
// An effort to ensure transmissions from various
// nodes do not happen too close together.
// Value is in milliseconds.
#define maxTransmitInterval 3000

// Heartbeats confirm the readiness of a given node.
// That concept can also be used to broadcast status
// values at a given interval.
// Value is in milliseconds.
#define heartbeatInterval 30000
long heartbeat = 0;

// Identify analog input pins
#define Pin_SolarCell A0
#define Pin_Battery A1
#define Pin_SoilMoisture A2

// Digital output pins that issue relay-control pulses.
// Relay is activated when when pin is HIGH.
// Relay is deactivated when pin is LOW.
// Relay can be normally-on or nomally-off, depending on how it is wired.
// Notation: RelayControl_<which pair>_<INx><blank><which MKR digital pin>
#define RelayControl_1_1 1
#define RelayControl_1_2 2
#define RelayControl_2_1 3
#define RelayControl_2_2 4

// Forward function definitions
typedef void (*Apparatus_Handler)(RequestResponseParameters *theseParameters, uint8_t index);
void LED_Handler(RequestResponseParameters *theseParameters, uint8_t index);
void PulseValve_Handler(RequestResponseParameters *theseParameters, uint8_t index);
void SolarCell_Handler(RequestResponseParameters *theseParameters, uint8_t index);
void Battery_Handler(RequestResponseParameters *theseParameters, uint8_t index);
void SoilMoisture_Handler(RequestResponseParameters *theseParameters, uint8_t index);
void Wait(long milliseconds);
void SendPositivePulse();
void SendNegativePulse();
float GetVWC(float volts);

// The list of apparatus connected to this node.
// A bit clutzy but trying to avoid using a formal list.
#define numApparatus 5
struct Apparatus
{
  char apparatusName[apparatusNameMaxLength]; // name unique to this node
  Apparatus_Handler thisHandler;              // function that deals with a given apparatus
  float status;                               // most recent setting/reading
  long timer;                                 // for actuators, length of time setting is to be active
};
Apparatus ApparatusList[numApparatus] =
{
  {"LED", LED_Handler, 0.0f, 0},
  {"PulseValve", PulseValve_Handler, 0.0f, 0},
  {"SolarCell", SolarCell_Handler, NAN, 0},
  {"Battery", Battery_Handler, NAN, 0},
  {"SoilMoisture", SoilMoisture_Handler, NAN, 0}
};


void setup()
{
  #ifdef DEBUG
    // Initialize serial port
    Serial.begin(9600);
    while (!Serial) Wait(1000); // wait for serial port to be ready
    Serial.println("\nDevice is active");
  #else
    Serial.begin(9600);
    Wait(1000); // give time for device to settle
  #endif

  // Initialize message-handling library.
  MessagingLibrary = new LoRaMessageHandler(localAddress);
  #ifdef DEBUG
    Serial.println("Transceiver is active (Node " + String(localAddress) + ")\n");
  #endif

  // Enable relay-control digital pins.
  // All digital pins are occupied.
  digitalWrite(RelayControl_1_1, LOW);
  digitalWrite(RelayControl_1_2, LOW);
  digitalWrite(RelayControl_2_1, LOW);
  digitalWrite(RelayControl_2_2, LOW);
  pinMode(RelayControl_1_1, OUTPUT); 
  pinMode(RelayControl_1_2, OUTPUT); 
  pinMode(RelayControl_2_1, OUTPUT); 
  pinMode(RelayControl_2_2, OUTPUT); 

  // Initialize digital pin LED_BUILTIN as an output.
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, OUTPUT);

  // Set all actuators to inactive status.
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(RelayControl_1_1, LOW);
  digitalWrite(RelayControl_1_2, LOW);
  digitalWrite(RelayControl_2_1, LOW);
  digitalWrite(RelayControl_2_2, LOW);
  
  // Configure analog digital conversion (ADC).
  // MKR WAN 1310 is a SAMD board.
  // Default reference is given by AR_DEFAULT.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReference
  // Capable of 12-bit ADC resolution.
  // This yields ADC output of 0 .. 4095.
  // https://docs.arduino.cc/language-reference/en/functions/analog-io/analogReadResolution
  analogReference(AR_DEFAULT);
  analogReadResolution(12);

  // Configure analog input pins.
  pinMode(Pin_SolarCell, INPUT);
  pinMode(Pin_Battery, INPUT);
  pinMode(Pin_SoilMoisture, INPUT);

  // Initialize the random number generator.
  // localAddress is unique to each node.
  randomSeed(localAddress);

  // Initialize the heartbeat.
  heartbeat = millis();
  
  // Ready
  #ifdef DEBUG
    Serial.println("===============================================");
    Serial.println("Arduino MKR WAN 1310 LoRa Apparatus Controller");
    Serial.println("===============================================");
  #endif
}

void loop()
{
  // Check for an incoming packet for this node.
  const uint8_t* thisMessage = NULL;
  if(MessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message.
    thisMessage = MessagingLibrary->getMESSAGE();
    #ifdef DEBUG
      Serial.print("\nReceived LoRa packet");
      Serial.print("  from Node " + String(thisMessage[LOCATION_SOURCE_ID]));
      Serial.println("  Message Type " + String(thisMessage[LOCATION_MESSAGE_TYPE]));
    #endif
  
    // Respond to request messages.
    // For now, ignore all other message types.
    if(thisMessage[LOCATION_MESSAGE_TYPE] == REQUEST_MSG)
    {
      // Extract the request.
      RequestResponseParameters thisResponse;
      memcpy(&thisResponse, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(thisResponse));
  
      if(strcmp(thisResponse.apparatusName, "Every") == 0) // request for all apparatus status
      {
        thisResponse.destination = thisMessage[LOCATION_SOURCE_ID];
        float settingValue = thisResponse.associatedValueA;
        for(uint8_t a = 0; a < 4; a++)
        {
          ApparatusList[a].thisHandler(&thisResponse, a);
          strcpy(thisResponse.apparatusName, ApparatusList[a].apparatusName);
          thisResponse.associatedValueA = ApparatusList[a].status;
          Wait(random(1000,maxTransmitInterval));
          MessagingLibrary->SendResponse(thisResponse);
          thisResponse.associatedValueA = settingValue;
          #ifdef DEBUG
            Serial.print("Sent response to request for status of ");
            Serial.print(ApparatusList[a].apparatusName);
            Serial.println(" to node " + String(thisResponse.destination));
          #endif
        }
      }
      else // request for action on specific apparatus
      {
        #ifdef DEBUG
          Serial.println("Looking for: " + String(thisResponse.apparatusName));
        #endif
        uint8_t a = 0;
        for( ; a < numApparatus; a++)
        {
          #ifdef DEBUG
            //Serial.print("  Comparing with: ");Serial.println(ApparatusList[a].apparatusName);
          #endif
          if(strcmp(thisResponse.apparatusName, ApparatusList[a].apparatusName) == 0)
            break;
        }
        if(a < numApparatus)
        {
          ApparatusList[a].thisHandler(&thisResponse, a);
          thisResponse.destination = thisMessage[LOCATION_SOURCE_ID];
          thisResponse.associatedValueA = ApparatusList[a].status;
        }
        else strcmp(thisResponse.apparatusName, (String("Unknown: ") + String(thisResponse.apparatusName)).c_str());
        Wait(random(1000,maxTransmitInterval));
        MessagingLibrary->SendResponse(thisResponse);
      }
    }
    else
    {
      // not a request message
      #ifdef DEBUG
        Serial.println("* Not a request message");
      #endif
    }
  }

  // Check the heartbeat.
  // Issue appropriate messages after a given interval.
  // Illustrates an alternative means of sending data.
  // In this case, we only transmit current sensor values.
  // One could also cause an actuator to change state periodically.
  // Text message contains the apparatus current status.
  // These messages are formatted as expected by the basestation.
  // Status,<apparatus name>,<this node's address>,<0: sensor, 1: actuator>,<status value>
  if((millis() - heartbeat) > heartbeatInterval)
  {
    // Battery
    float reading =
      2.0f * (float)analogRead(Pin_Battery) * voltageReference / analogResolution;
    String text = "Status,Battery," + String(localAddress) + ",0," + String(reading);
    MessagingLibrary->SendTextMessage(text, basestationAddress);
    Wait(random(1000,maxTransmitInterval));

    //Solar Cell
    reading =
      2.0f * (float)analogRead(Pin_SolarCell) * voltageReference / analogResolution;
    text = "Status,SolarCell," + String(localAddress) + ",0," + String(reading);
    MessagingLibrary->SendTextMessage(text, basestationAddress);
    Wait(random(1000,maxTransmitInterval));

    // Soil Moisture (voltage splitter not needed with this sensor)
    reading =
      (float)analogRead(Pin_SoilMoisture) * voltageReference / analogResolution;
    reading = GetVWC(reading);
    text = "Status,SoilMoisture," + String(localAddress) + ",0," + String(reading);
    MessagingLibrary->SendTextMessage(text, basestationAddress);

    #ifdef DEBUG
      Serial.println("\nSent heartbeat: " + text);
    #endif
    heartbeat = millis();
  }
}

// ============= Function Definitions =================

// The following functions handle requests for
// the various apparatuses connected to this particular node.

void LED_Handler(RequestResponseParameters *theseParameters, uint8_t index)
{
  theseParameters->isActuator = true;
  if(theseParameters->associatedValueA == theseParameters->associatedValueA)
  {
    if(theseParameters->associatedValueA <= 0.0f)
    {
      ApparatusList[index].status = 0.0f;
      digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
      ApparatusList[index].status = 1.0f;
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

void PulseValve_Handler(RequestResponseParameters *theseParameters, uint8_t index)
{
  theseParameters->isActuator = true;
  if(theseParameters->associatedValueA == theseParameters->associatedValueA)
  {
    if(theseParameters->associatedValueA <= 0.0f)
    {
      ApparatusList[index].status = 0.0f;
      SendNegativePulse();
    }
    else
    {
      ApparatusList[index].status = 1.0f;
      SendPositivePulse();
    }
  }
}

void Battery_Handler(RequestResponseParameters *theseParameters, uint8_t index)
{
  theseParameters->isActuator = false;
  ApparatusList[index].status =
    2.0f * (float)analogRead(Pin_Battery) * voltageReference / analogResolution;
}

void SolarCell_Handler(RequestResponseParameters *theseParameters, uint8_t index)
{
  theseParameters->isActuator = false;
  ApparatusList[index].status =
    2.0f * (float)analogRead(Pin_SolarCell) * voltageReference / analogResolution;
}

void SoilMoisture_Handler(RequestResponseParameters *theseParameters, uint8_t index)
{
  theseParameters->associatedValueA = ApparatusList[index].status =
    (2.0f * (float)analogRead(Pin_SoilMoisture) * voltageReference) / analogResolution;
  theseParameters->associatedValueA = ApparatusList[index].status =
    GetVWC(ApparatusList[index].status);
  theseParameters->isActuator = false;
  strcpy(theseParameters->apparatusName, ApparatusList[index].apparatusName);
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

// Emit a positive pulse to the pulse-controlled valve.
// This sets the valve to its ON position.
// Would have to be expanded if there is more than one such valve.
void SendPositivePulse()
{
  // Assumes correct wiring per book's discussion.
  digitalWrite(RelayControl_1_1, HIGH);
  digitalWrite(RelayControl_2_1, HIGH);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #1 LED should be ON");
    Serial.println("Relay Pair #2 Relay #1 LED should be ON");
    Serial.println("Voltage should read +5vdc\n");
  #endif
  Wait(5000); // approximate pulse length in milliseconds
  digitalWrite(RelayControl_1_1, LOW);
  digitalWrite(RelayControl_2_1, LOW);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #1 LED should be OFF");
    Serial.println("Relay Pair #2 Relay #1 LED should be OFF");
    Serial.println("Voltage should read +/-0vdc\n");
  #endif
}

// Emit a negative pulse to the pulse-controlled valve.
// This sets the valve to its OFF position.
// Would have to be expanded if there is more than one such valve.
void SendNegativePulse()
{
  // Assumes correct wiring per book's discussion.
  digitalWrite(RelayControl_1_2, HIGH);
  digitalWrite(RelayControl_2_2, HIGH);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #2 LED should be ON");
    Serial.println("Relay Pair #2 Relay #2 LED should be ON");
    Serial.println("Voltage should read -5vdc\n");
  #endif
  Wait(5000); // approximate pulse length in milliseconds
  digitalWrite(RelayControl_1_2, LOW);
  digitalWrite(RelayControl_2_2, LOW);
  #ifdef DEBUG
    Serial.println("Relay Pair #1 Relay #2 LED should be OFF");
    Serial.println("Relay Pair #2 Relay #2 LED should be OFF");
    Serial.println("Voltage should read +/-0vdc\n");
  #endif
}

// Takes voltage as input, locates it in the table, applies the equation of a line, and delivers the result.
// In this case, the result is VWC.
float GetVWC(float volts)
{
  const long numRows = 13;
  float table[numRows][2] =
  { // (x_volts, y_VWC)
    {0.0f,  0.0f},
    {0.1f,  0.1f},
    {0.6f,  5.0f},
    {1.1f, 10.0f},
    {1.3f, 15.0f},
    {1.4f, 20.0f},
    {1.5f, 25.0f},
    {1.6f, 30.0f},
    {1.7f, 35.0f},
    {1.8f, 40.0f},
    {2.0f, 45.0f},
    {2.3f, 50.0f},
    {3.0f, 60.0f}
  };

  long r;
  for (r = 1; r < numRows; r++)
  {
    if ((volts >= table[r - 1][0]) &&
      (volts <= table[r][0]))
      break;
  }

  if (r == numRows) return (float)NULL;
  else
  {
    float slope =
      (table[r][1] - table[r - 1][1]) / (table[r][0] - table[r - 1][0]);

    float VWC = slope * (volts - table[r - 1][0]) + table[r - 1][1];

    return VWC;
  }
}
