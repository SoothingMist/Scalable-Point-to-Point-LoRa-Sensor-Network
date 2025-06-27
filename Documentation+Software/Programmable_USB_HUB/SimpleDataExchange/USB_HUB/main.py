
# This program looks for two serial ports.
# Data from one is sent to the other.
# Thus, it is possible to send data from one device to another
# when those two devices cannot be directly connected.
#
# This particular version is meant only for very short messages.
# Maximum message length should be 63 bytes or less due to Arduino buffer limits.
# Messages are assumed to start with the total number of bytes in the message.
#
# This idea could be extended to more than two devices.

# Required for listing serial ports
# https://pypi.org/project/pyserial
import serial.tools.list_ports

# Required to keep track of time.
import time

# Require to access operating system
import sys

# ==================================================
# These are constants and should not be changed.

# Constants associated with the two serial ports.
# "None" values are set during configuration.
PORT_A = None
PORT_A_NAME = 'COM15' # Windows
#PORT_A_NAME = '/dev/ttyACM1' # Linux
PORT_A_BAUD_RATE = 9600
PORT_B = None
PORT_B_NAME = 'COM16' # Windows
# PORT_B_NAME = '/dev/ttyACM0' # Linux
PORT_B_BAUD_RATE = 9600
TRANSCEIVER_PORT = None

# Message configuration settings
HANDSHAKE = b'H'
ZERO = bytes([0])
TWO = bytes([2])
MAX_MESSAGE_LENGTH = 63
MESSAGE = [bytes([0])] * MAX_MESSAGE_LENGTH
SLEEP_STANDARD = 4 # seconds to wait between interaction with devices

# ==================================================

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

# Get the next message
def GetNextMessage(thisPort):
  while thisPort.in_waiting < 1: time.sleep(SLEEP_STANDARD) # wait for data
  MESSAGE[0] = thisPort.read() # read number of bytes in message
  for i in range(1, int.from_bytes(MESSAGE[0], sys.byteorder)):
    MESSAGE[i] = thisPort.read() # https://www.w3schools.com/python/python_for_loops.asp

# Send a message
def SendMessage(thisPort):
  # https://stackoverflow.com/questions/34009653/convert-bytes-to-int
  # https://www.w3schools.com/python/python_for_loops.asp
  for i in range(0, int.from_bytes(MESSAGE[0], sys.byteorder)): thisPort.write(MESSAGE[i])

# Exchange handshakes between the two devices
def DoHandshakes():
  global TRANSCEIVER_PORT

  # Wait for connection HANDSHAKE from the transceiver.
  # When received, send on to the data device.
  # This process accommodates devices that spew garbage upon startup.
  print("\nLooking for connection handshakes:")
  connected = False
  while not connected:
    if PORT_A.in_waiting:
      MESSAGE[0] = ZERO
      while MESSAGE[0] != TWO: MESSAGE[0] = PORT_A.read()
      MESSAGE[1] = PORT_A.read()
      if MESSAGE[1] == HANDSHAKE:
        SendMessage(PORT_B)
        print("PORT_A sent handshake to Port_B.")
        MESSAGE[0] = ZERO
        MESSAGE[1] = ZERO
        while MESSAGE[0] != TWO:
          MESSAGE[0] = PORT_B.read()
        while MESSAGE[1] != HANDSHAKE:
          MESSAGE[1] = PORT_B.read()
        SendMessage(PORT_A)
        print("PORT_A received handshake from Port_B.")
        TRANSCEIVER_PORT = PORT_A
        connected = True
    else:
      if PORT_B.in_waiting:
        MESSAGE[0] = ZERO
        while MESSAGE[0] != TWO: MESSAGE[0] = PORT_B.read()
        MESSAGE[1] = PORT_B.read()
        if MESSAGE[1] == HANDSHAKE:
          SendMessage(PORT_A)
          print("PORT_B sent handshake to Port_A.")
          MESSAGE[0] = ZERO
          MESSAGE[1] = ZERO
          while MESSAGE[0] != TWO: MESSAGE[0] = PORT_A.read()
          while MESSAGE[1] != HANDSHAKE: MESSAGE[1] = PORT_A.read()
          SendMessage(PORT_B)
          print("PORT_B received handshake from Port_A.")
          TRANSCEIVER_PORT = PORT_B
          connected = True

# Main Process
if __name__ == '__main__':
  
  # Use this if uncertain as to which serial ports are active
  FindSerialPorts()
  #exit(0)

  print("Configuring serial ports...")

  # Connect to PORT_A
  try:
    PORT_A = serial.Serial(PORT_A_NAME, PORT_A_BAUD_RATE)
  except Exception as e:
    print(f'\nCaught {type(e)}:\n', e)  # https://docs.python.org/3/tutorial/errors.html
    print("\nIs the PORT_A device connected and active?")
    print("Has PORT_A's name and baudrate been correctly specified?")
    print("Is PORT_A's IDE Serial Monitor deactivated?")
    print("Exiting program.\n")
    exit(1)
  time.sleep(SLEEP_STANDARD)  # wait long enough for the device to be ready
  # Get the "I am ready" message here, if there is to be one.
  # https://realpython.com/python-print/#preventing-line-breaks
  # https://stackoverflow.com/questions/14292746/how-to-python-convert-bytes-to-readable-ascii-unicode
  print("Connected to PORT_A.")
  print('\tPort Name:\t\t', PORT_A.name)
  print('\tPort Baudrate:\t', PORT_A.baudrate)

  # Connect to PORT_B
  try:
    PORT_B = serial.Serial(PORT_B_NAME, PORT_B_BAUD_RATE)
  except Exception as e:
    print(f'\nCaught {type(e)}:\n', e)  # https://docs.python.org/3/tutorial/errors.html
    print("\nIs the PORT_B device connected and active?")
    print("Has PORT_B's name and baudrate been correctly specified?")
    print("Is PORT_B's IDE Serial Monitor deactivated?")
    print("Exiting program.\n")
    exit(1)
  time.sleep(SLEEP_STANDARD)  # wait long enough for the device to be ready
  # Get the "I am ready" message here, if there is to be one.
  # https://realpython.com/python-print/#preventing-line-breaks
  # https://stackoverflow.com/questions/14292746/how-to-python-convert-bytes-to-readable-ascii-unicode
  print("Connected to PORT_B.")
  print('\tPort Name:\t\t', PORT_B.name)
  print('\tPort Baudrate:\t', PORT_B.baudrate)

  # The two devices wait for a handshake byte.
  DoHandshakes()
  print("Handshakes exchanged\n") # the devices are now talking to each other

  # Look for and pass messages between the two devices.
  while True:

    # Check PORT_A
    if PORT_A.in_waiting > 0:
      GetNextMessage(PORT_A)
      #=======
      # This is where something can be done with the data flow.
      # For this demonstration, we assume text messages.
      # But that is not a necessary assumption.
      print("{:.2f}".format(time.time()), " > PORT_A message length: ",
            int.from_bytes(MESSAGE[0], sys.byteorder), end = "; ")
      for c in range(1, int.from_bytes(MESSAGE[0], sys.byteorder)):
        print(MESSAGE[c].decode('ascii'), end = "")
      print()
      #=======
      SendMessage(PORT_B)

    # Check PORT_B
    if PORT_B.in_waiting > 0:
      GetNextMessage(PORT_B)
      # =======
      # This is where something can be done with the data flow.
      # For this demonstration, we assume text messages.
      # But that is not a necessary assumption.
      print("{:.2f}".format(time.time()), " > PORT_A message length: ",
            int.from_bytes(MESSAGE[0], sys.byteorder), end="; ")
      for c in range(1, int.from_bytes(MESSAGE[0], sys.byteorder)):
        print(MESSAGE[c].decode('ascii'), end="")
      print()
      # =======
      SendMessage(PORT_A)
