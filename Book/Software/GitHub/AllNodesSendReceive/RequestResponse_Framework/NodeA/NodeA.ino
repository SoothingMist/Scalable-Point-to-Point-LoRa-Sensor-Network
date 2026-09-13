/*
  LoRa Request/Response Example

  Sends a request at random intervals.
  Checks for response.

  A table of outstanding requests is maintained.
  Reacts to lack of response.
  
  Checks for requests.
  Sends response to requests.

  This program's concept works best if only one
  request is made at a time of a given apparatus
  at a given destination.

  Nothing actually happens to satisfy requests
  in this basic example. However, a response
  is generated for every request.
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
LoRaMessageHandler *MessagingLibrary = NULL;

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
// For this demonstration, we assume exactly two items in the list.
String apparatusList[] =
{
  "Sensor <name>",
  "Actuator <name>"
};

// List of outstanding requests
struct CurrentRequest
{
  uint8_t apparatusID;
  uint32_t associatedValue;
  uint8_t destination;
  long timeSent;
};
SingleLinkedList<CurrentRequest> outstandingRequests;

byte msgCount = 0;              // count of outgoing messages
long lastSendTime = 0;          // last send time
const long maxInterval = 5000;  // maximum millisecond interval between sends
long interval = 0;              // current interval between sends

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
  MessagingLibrary = new LoRaMessageHandler(localAddress);
  #ifdef DEBUG
    Serial.println("Transceiver is active (Node " + String(localAddress) + ")");
  #endif
  
  // ================ End Standard Setup ===========================

  // Initialize program-specific variables.
  randomSeed(localAddress);
  interval = random(maxInterval);
  lastSendTime = millis();

  // Ready
  #ifdef DEBUG
    Serial.println("====================================================");
    Serial.println("Arduino MKR WAN 1310 LoRa Request/Response Framework");
    Serial.println("====================================================");
  #endif
}

void loop()
{
  // There are two types of apparatus,
  // sensor and actuator. This variable
  // allows for selecting one or the other
  // for generating a request.
  static uint16_t apparatusSelector = 0;
  
  // Check if it is time to send another request
  if (millis() - lastSendTime > interval)
  {
    // Compose message
    CurrentRequest thisRequest;
    thisRequest.destination = destinationAddress;
    apparatusSelector++;

    // Select an apparatus to make a request about.
    // (Flip/Flops between two.)
    if(apparatusSelector % 2 == 0) // apparatusSelector is an even number
    {
      // Send request for sensor reading
      // (sensor ID, dummy value, destination)
      thisRequest.apparatusID = 0;
      thisRequest.associatedValue = 5; // interpret for type of request
    }
    else // apparatusSelector is an odd number
    {
      // Send request for activation
      // (actuator ID, length of activation, destination)
      thisRequest.apparatusID = 1;
      thisRequest.associatedValue = 10; // interpret for type of request
    }

    // Broadcast the request

    #ifdef DEBUG
      Serial.println("\nSending Request. Node " + String(thisRequest.destination) +
                     ". Apparatus: " + apparatusList[thisRequest.apparatusID] +
                     ". Associated Value: " + String(thisRequest.associatedValue) + ".");
    #endif

    MessagingLibrary->SendRequest(thisRequest.apparatusID, thisRequest.associatedValue, thisRequest.destination);
    
    #ifdef DEBUG
      Serial.println("Sent Request");
    #endif

    // Add to list of outstanding requests
    thisRequest.timeSent = millis();
    outstandingRequests.add(thisRequest);
    #ifdef DEBUG
      Serial.println("Number Outstanding Requests: " + String(outstandingRequests.getSize()));
    #endif

    // Select the next time to send a request
    lastSendTime = millis();
    interval = random(maxInterval);
  }

  // Check for an incoming packet for this node
  if(MessagingLibrary->CheckForIncomingPacket() > 0)
  {
    // Get a pointer to the received message
    const uint8_t* thisMessage = MessagingLibrary->getMESSAGE();
    
    // ToDo: Deduce the type of message.
    // React or respond as appropriate.
    // Requests, Responses, Unknowns
    if(thisMessage[LOCATION_MESSAGE_TYPE] == 1) // received a request message
    {
      uint32_t associatedValue = 0; // determine a request-appropriate value
      memcpy(&associatedValue, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(uint32_t));
      
      #ifdef DEBUG
        Serial.println("\nReceived Request From Node " + 
                       String(thisMessage[LOCATION_SOURCE_ID]));
        Serial.println("Regarding " + apparatusList[thisMessage[LOCATION_APPARATUS_ID]]);
        Serial.println("Associated Value: " + String(associatedValue));
      #endif
      // Just echoing associatedValue for now
      MessagingLibrary->SendResponse(thisMessage[LOCATION_APPARATUS_ID], associatedValue,
                                     thisMessage[LOCATION_SOURCE_ID]);
    }
    else if(thisMessage[LOCATION_MESSAGE_TYPE] == 2) // received a response message
    {
      uint32_t associatedValue = 0; // determine a request-appropriate value
      memcpy(&associatedValue, thisMessage + MESSAGE_HEADER_LENGTH, sizeof(uint32_t));
      
      #ifdef DEBUG
        Serial.println("\nReceived Response From Node " + 
                       String(thisMessage[LOCATION_SOURCE_ID]));
        Serial.println("Regarding " + apparatusList[thisMessage[LOCATION_APPARATUS_ID]]);
        Serial.println("Associated Value: " + String(associatedValue));
      #endif
      
      // Remove requests from list of outstanding requests after a response has been received.
      bool found = false;
      for(int r = 0; r < outstandingRequests.getSize(); r++)
      {
        CurrentRequest thisRequest = outstandingRequests.get(r);
        if( (thisRequest.apparatusID == thisMessage[LOCATION_APPARATUS_ID]) &&
            (thisRequest.destination == thisMessage[LOCATION_SOURCE_ID]) )
        {
          found = true;
          outstandingRequests.remove(r);
          Serial.println("*** Removed satisfied outstanding request");
          break;
        }
      }
      if( ! found) Serial.println("*** Could not find associated request");
    }
    else Serial.println("*** Not equipped to deal with this message type");
  }
  
  // Check for outstanding requests that are not satisfied.
  while(outstandingRequests.getSize() > 0)
  {
    // Identify oldest outstanding request
    CurrentRequest oldestRequest = outstandingRequests.get(0);

    // React to outstanding requests if wait time is exceeded.
    // oldestRequest contains sufficient information for
    // making new request or sending a notification of a
    // non-responsive node. Delete old outstanding requests.
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
