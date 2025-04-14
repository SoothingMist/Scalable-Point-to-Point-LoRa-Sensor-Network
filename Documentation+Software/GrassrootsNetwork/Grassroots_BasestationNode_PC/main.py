
# Ref:
# https://www.geeksforgeeks.org/how-to-display-multiple-images-in-one-window-using-opencv-python
# https://answers.opencv.org/question/175912/how-to-display-multiple-images-in-one-window
# https://www.hackster.io/polyhedra64/how-to-link-arduino-serial-to-python-62b9a5

# Maybe interesting:
# https://stackoverflow.com/questions/50881227/display-images-in-a-grid

# Imported libraries
import serial, serial.tools.list_ports # https://pypi.org/project/pyserial
import time # https://docs.python.org/3/library/time.html

# Constants. These values should not be changed.

# Serial port to which the LoRa transceiver is connected
Serial_Port = None
SERIAL_PORT_NAME = 'COM6' # Windows
#Serial_PORT_NAME = '/dev/ttyACM1' # Linux
SERIAL_PORT_BAUD_RATE = 9600

MAX_MESSAGE_SIZE = 256 # LoRa packets are no longer than this
EXPECTED_FINAL_MESSAGE = "<done>"

# Location of elements in message vector
LOCATION_MESSAGE_INDEX  = 0
LOCATION_SYSTEM_ID      = 1
LOCATION_SOURCE_ID      = 2
LOCATION_DESTINATION_ID = 3
LOCATION_MESSAGE_ID     = 4
LOCATION_MESSAGE_TYPE   = 6
LOCATION_SENSOR_ID      = 7
LOCATION_REBROADCASTS   = 8
MESSAGE_HEADER_LENGTH   = 9

# List all serial ports that have an PORT_A microcontroller plugged in.
# https://be189.github.io/lessons/10/control_of_PORT_A_with_python.html#
def FindSerialPorts():
  print("\nSerial ports:")
  Ports = serial.tools.list_ports.comports()
  if Ports is not None:
    print("Detected ", len(Ports), " total ports")
    print("Name\t Device\t Manufacturer\t\t\t\t\t Description")
    for port in Ports:
        print(port.name, "\t", port.device, "\t", port.manufacturer, "\t", port.description)
  else: print("No ports found")
  print()

# Connect to the correct serial port.
def ConnectSerialPort(GENERIC_PORT_NAME):
  global Serial_Port
  # Identify the device connected to the specified port
  try:
    Serial_Port = serial.Serial(SERIAL_PORT_NAME, SERIAL_PORT_BAUD_RATE)
  except Exception as thisException:  # https://docs.python.org/3/tutorial/errors.html
    print(f'\nCaught {type(thisException)}:\n', thisException)
    print("\nIs the ", GENERIC_PORT_NAME, " device connected and active?")
    print("Has ", GENERIC_PORT_NAME, " name and baudrate been correctly specified?")
    print("Is the IDE Serial Monitor of the device connected to ", GENERIC_PORT_NAME, " been deactivated?")
    Serial_Port = None
    return
  time.sleep(5)  # wait long enough for the device to be ready

# Main Process
if __name__ == '__main__':

  # Use this if uncertain as to which serial ports are active
  FindSerialPorts()
  # exit(0)

  # Connect to the LoRaP2P microcontroller/transceiver
  print("Connecting to LoRaP2P microcontroller/transceiver (" + SERIAL_PORT_NAME + ")")
  ConnectSerialPort(SERIAL_PORT_NAME)
  if Serial_Port is None:
    print("Device is not connected.")
    print("Exiting program.\n")
    exit(1)

  # Keep receiving and acting on messages until <done> message is received
  print("Connected. Awaiting Messages...\n")
  notDone = True
  while notDone:
    # Receive next message (https://www.tutorialspoint.com/how-to-convert-bytes-to-int-in-python_
    message = Serial_Port.read(1) # first byte of message is last message vector index
    message += Serial_Port.read(message[0] - 1) # gets the rest of the message
    # Unpack the message
    if message[LOCATION_MESSAGE_TYPE] == 3: # Check for message type 3, a text message
      contents = ""
      for i in range(MESSAGE_HEADER_LENGTH, message[0]): contents += chr(message[i])
      if contents == EXPECTED_FINAL_MESSAGE: notDone = False
      else: print(str(time.time()) + ": " +  contents)
      # parse here and take appropriate action
    else:
      print("Unknown message type: ", end = "")
      print(message[LOCATION_MESSAGE_TYPE])

  # Finished
  print("\nFinished.\n")
  Serial_Port.close()
  Serial_Port = None
  exit(0)
