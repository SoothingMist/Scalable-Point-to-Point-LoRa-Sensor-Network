/*
  LoRa Request/Response Example

  Sends a request at random intervals.
  Checks for response.

  A table of outstanding requests is maintained.
  Reacts to lack of response.

  Checks for requests.
  Sends response to requests.

  Ensures only one request is made at a time of a
  given apparatus at a given destination.

  This application of the RequestResponse_Framework
  sends and responds to requests for battery voltage. 
  It also sends and responds to requests to turn on/off
  the board's standard LED, which simulates an actuator.
*/

// For viewing Serial.println text on serial monitor.
#define DEBUG

// How long to wait for a response (milliseconds)
#define waitTime 60000

// Get the local and destination addresses for this node.
// Destination can be made variable.
#include "Addresses.h"

// Library for message handling.
// Includes LoRa library.
#include "LoRaMessageHandler.h"
LoRaMessageHandler *messagingLibrary = NULL;

// Library for implementing Lists on Arduino
// https://nkaaf.github.io/Arduino-List/html/index.html
#include <SingleLinkedList.hpp>

// Sensor/Actuator (Apparatus) List.
// apparatusID is the position in the list.
// All request/response nodes need to
// have the same ID associations.
// To find the number of strings in such
// a list within the scope of the declaration,
// use, for example:
// numApparatus = sizeof(apparatusList) / sizeof(apparatusList[0]).
String apparatusList[] =
{
  "Sensor BATTV",
  "Sensor SOIL",
  "Actuator LED"
};
static long numApparatus = sizeof(apparatusList) / sizeof(apparatusList[0]);
// Two types of apparatus for this demo
#define unknown 255
#define sensor 1
#define actuator 0

// List of outstanding requests
struct CurrentRequest
{
  uint8_t apparatusID;
  uint32_t associatedValue;
  uint8_t destination;
  long timeSent;
};
SingleLinkedList<CurrentRequest> outstandingRequests;

// Timing variables
long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // current interval between sends

// External subroutines for responding to requests
extern float ReadVoltage(uint8_t voltagePin);

// Output pins for controlling actuators
#define ledPin LED_BUILTIN

// Voltage input pins.
// Use jumper wires to connect input pins
// to the voltages to be measured. Beware of
// voltage limitations. For example, the
// Arduino MKR WAN 1310 voltage input is limited
// to 3.3 volts. Going beyond that will ruin
// the device.
#define batteryPin A2
#define soilPin A1

void setup()
{
  // ============ Standard MKR WAN 1310 Setup Code =============
  
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) wait(100); // wait for serial port to be ready
  #ifdef DEBUG
    Serial.println("Node is active");
  #endif
  
  // Initialize message-handling library.
  messagingLibrary = new LoRaMessageHandler(localAddress);
  #ifdef DEBUG
    Serial.println("Transceiver is active (Node " + String(localAddress) + ")");
  #endif
  
  // ================ End Standard Setup ===========================

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();

  // Initialize actuator control pins.
  // Actuators initialize to LOW, OFF.
  pinMode(ledPin, OUTPUT); digitalWrite(ledPin, LOW);

  // Ready
  #ifdef DEBUG
    Serial.println("========================================================");
    Serial.println("Arduino MKR WAN 1310 LoRa Request/Response Demonstration");
    Serial.println("========================================================");
  #endif
}

void loop()
{
  // This variable allows for selecting an apparatus.
  // A request is then generated for that apparatus.
  static uint8_t apparatusSelector = 0;
  
  // Check if it is time to send another request
  if (millis() - lastSendTime > interval)
  {
    // Compose message
    CurrentRequest thisRequest;
    thisRequest.destination = destinationAddress;
    bool broadcast = true;

    // Select an apparatus to make a request about
    apparatusSelector++;
    uint8_t thisApparatus = apparatusSelector % numApparatus;
    thisRequest.apparatusID = thisApparatus;
    uint8_t apparatusType = 0; // only sensors and actuators for now
    if(apparatusList[thisApparatus].indexOf("Sensor") >= 0) apparatusType = sensor;
    else if(apparatusList[thisApparatus].indexOf("Actuator") >= 0) apparatusType = actuator;
    else apparatusType = unknown;
    switch(apparatusType)
    {
      case sensor: // send request for sensor reading
        thisRequest.associatedValue = 0; // request for reading
        break;
      case actuator: // send request for actuator
        thisRequest.associatedValue = 0; // request to toggle
        break;
      default: // apparatus nomentclature not understood
        #ifdef DEBUG
          Serial.println("Error: Apparatus nomenclature not understood");
          Serial.println("Apparatus # " + String(thisApparatus) +
                         ". Nomenclature: " + apparatusList[thisApparatus]);
        #endif
        broadcast = false;
    }

    // Broadcast the request
    if(broadcast)
    {
      #ifdef DEBUG
        Serial.println("\nSending Request to Node " + String(thisRequest.destination));
        Serial.println("Apparatus: " + apparatusList[thisRequest.apparatusID]);
        Serial.print("Associated Value: ");
        Serial.println(thisRequest.associatedValue);
      #endif
  
      // Send request and add to list of outstanding requests.
      // Reject request if destination/apparatus already occupied.
      float r = FindRequest(thisRequest.apparatusID, thisRequest.destination);
      if(r < 0.0f)
      {
        messagingLibrary->SendRequest(thisRequest.apparatusID, thisRequest.associatedValue, thisRequest.destination);
        #ifdef DEBUG
          Serial.println("Sent Request");
        #endif
        thisRequest.timeSent = millis();
        outstandingRequests.add(thisRequest);
        #ifdef DEBUG
          Serial.println("Outstanding Requests: " + String(outstandingRequests.getSize()));
        #endif
      }
      #ifdef DEBUG
        else Serial.println("*** Apparatus already engaged");
      #endif
    }
      
    // Select the next time to send a request
    lastSendTime = millis();
    interval = random(maxInterval);
  }

  // Check for an incoming packet for this node
  if(messagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = messagingLibrary->getMESSAGE();

    // Working only with request/response message types
    if((thisMessage[LOCATION_MESSAGE_TYPE] == 1) ||
       (thisMessage[LOCATION_MESSAGE_TYPE] == 2))
    {
      // Need to check thisMessage[LOCATION_APPARATUS_ID] for valid ID.
      // Ok this way for this demo.
      bool broadcast = true;
      uint8_t apparatusType = 0; // only sensors and actuators for now
      if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("Sensor") >= 0) apparatusType = sensor;
      else if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("Actuator") >= 0) apparatusType = actuator;
      else apparatusType = unknown;

      // Deduce the type of message. React as appropriate.
      if(thisMessage[LOCATION_MESSAGE_TYPE] == 1) // received a request message
      {
        bool sendPacket = true;
        uint32_t associatedValue = 0;
        switch(apparatusType) // sensor request
        {
          case sensor: // request regarding a sensor
            Serial.println("\nReceived sensor request.");
            Serial.println("Node " + String(thisMessage[LOCATION_SOURCE_ID]) +
                           ". Regarding '" + apparatusList[thisMessage[LOCATION_APPARATUS_ID]] + "'");
            memcpy(&associatedValue, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(associatedValue));
            if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("BATTV") >= 0)
            {
              if(associatedValue == 0) // only dealing with requests for readings for now
              {
                float voltage = ReadVoltage(batteryPin);
                voltage *= 2.0f;
                memcpy(&associatedValue, &voltage, sizeof(associatedValue));
              }
              else
              {
                #ifdef DEBUG
                  Serial.println("*** invalide sensor request");
                #endif
                broadcast = false;
              }
            }
            else if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("SOIL") >= 0)
            {
              if(associatedValue == 0) // only dealing with requests for readings for now
              {
                float voltage = ReadVoltage(soilPin);
                float VWC = GetVWC(voltage);
                memcpy(&associatedValue, &VWC, sizeof(associatedValue));
              }
              else
              {
                #ifdef DEBUG
                  Serial.println("*** invalid sensor request");
                #endif
                broadcast = false;
              }
            }
            else
            {
              #ifdef DEBUG
                Serial.println("*** unrecognized sensor");
              #endif
              broadcast = false;
            }
  
            break;
  
          case actuator: // actuator request
            Serial.println("\nReceived actuator request");
            Serial.print("Node: " + String(thisMessage[LOCATION_SOURCE_ID]) +
                         "; Actuator: " + String(thisMessage[LOCATION_APPARATUS_ID]));
            if(digitalRead(ledPin) == HIGH)
            {
              digitalWrite(ledPin, LOW);
              associatedValue = 0;
              Serial.println(". New Status: OFF");
            }
            else
            {
              digitalWrite(ledPin, HIGH);
              associatedValue = 1;
              Serial.println(". New Status: ON");
            }
            break;
  
          default: // apparatus nomentclature not understood
            #ifdef DEBUG
              Serial.println("Error: Requested apparatus nomenclature not understood");
              Serial.println("Apparatus # " + String(thisMessage[LOCATION_APPARATUS_ID]) +
                             ". Nomenclature: " + apparatusList[thisMessage[LOCATION_APPARATUS_ID]]);
            #endif
            broadcast = false;
        }
  
        // Send response
        if(broadcast)
          messagingLibrary->SendResponse(thisMessage[LOCATION_APPARATUS_ID], associatedValue,
                                         thisMessage[LOCATION_SOURCE_ID]);
      }
      else if(thisMessage[LOCATION_MESSAGE_TYPE] == 2) // received a response message
      {
        boolean remove = true;
        switch(apparatusType)
        {
          case sensor:
            if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("BATTV") >= 0)
            {
              float voltage = 0.0f;
              memcpy(&voltage, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(voltage));
              Serial.println("\nReceived sensor response.");
              Serial.println("Node: " + String(thisMessage[LOCATION_SOURCE_ID]) +
                             ". Battery Volts: " + String(voltage));
            }
            else if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("SOIL") >= 0)
            {
              float VWC = 0.0f;
              memcpy(&VWC, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(VWC));
              Serial.println("\nReceived sensor response.");
              Serial.println("Node: " + String(thisMessage[LOCATION_SOURCE_ID]) +
                             ". VWC: " + String(VWC));
            }
            #ifdef DEBUG
              else Serial.println("*** Unknown sensor");
            #endif
            
            break;
  
          case actuator:
            if(apparatusList[thisMessage[LOCATION_APPARATUS_ID]].indexOf("LED") >= 0)
            {
              uint32_t actuatorStatus;
              memcpy(&actuatorStatus, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(actuatorStatus));
              Serial.println("\nReceived actuator response");
              Serial.print("Node: " + String(thisMessage[LOCATION_SOURCE_ID]) +
                           "; Actuator: " + String(thisMessage[LOCATION_APPARATUS_ID]));
              if(actuatorStatus == 1) Serial.println("; Status: ON");
              else if(actuatorStatus == 0) Serial.println("; Status: OFF");
              else Serial.println(". Status: Unknown");
            }
            else Serial.println("*** Unknown actuator");
            break;
  
          default:
            Serial.println("*** Unrecognized apparatus: " + String(thisMessage[LOCATION_APPARATUS_ID]));
            remove = false;
        }
        
        // Remove request from list of outstanding requests after a response has been received and acted upon.
        if(remove)
        {
          float r = FindRequest(thisMessage[LOCATION_APPARATUS_ID], thisMessage[LOCATION_SOURCE_ID]);
          if(r >= 0.0f)
          {
              outstandingRequests.remove((int)r);
              Serial.println("*** Removed satisfied outstanding request");
          }
          else Serial.println("*** Could not find associated request");
        }
      }
      else Serial.println("*** Not equipped to deal with this message type");
    }
  }
  
  // Check for outstanding requests that are not satisfied.
  while(outstandingRequests.getSize() > 0)
  {
    // Identify oldest outstanding request
    CurrentRequest oldestRequest = outstandingRequests.get(0);

    // Delete if wait time is exceeded
    if(millis() - oldestRequest.timeSent > waitTime)
    {
      Serial.println("\n*** Old unanswered request. Deleting.");
      outstandingRequests.remove(0);
    }
    else break;
  }
}

// ============= Function Definitions =================

// Wait for a specific number of milliseconds.
// delay() is blocking so we do not use that.
// This approach does not use hardware-specific timers.
void wait(long milliseconds)
{
  long beginTime = millis();
  byte doSomething = 0;
  while ((millis() - beginTime) <= milliseconds) doSomething++;
}

// Find a destination/apparatus pair in the list of outstanding requests.
float FindRequest(uint8_t apparatus, uint8_t sourceID)
{
  int r;
  for(r = 0; r < outstandingRequests.getSize(); r++)
  {
    CurrentRequest thisRequest = outstandingRequests.get(r);
    if( (thisRequest.apparatusID == apparatus) &&
        (thisRequest.destination == sourceID) )
    {
      break;
    }
  }

  //Serial.println("FindRequest: " + String(r) + " / " + String(outstandingRequests.getSize()));
  if(r == outstandingRequests.getSize()) return -1.0f;
  else return (float)r;
}

// Takes voltage as input, locates it in the table, applies the equation of a line, and delivers VWC.
// Calibrated for a specific soil type.
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
