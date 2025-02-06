
# https://realpython.com/intro-to-python-threading
# https://www.pythontutorial.net/python-concurrency/python-threading-lock

import logging # https://docs.python.org/3.10/library/logging.html
import threading # https://docs.python.org/3.10/library/threading.html
import time # https://docs.python.org/3.10/library/time.html
import queue # https://www.guru99.com/python-queue-example.html, https://docs.python.org/3.10/library/queue.html
import sys # Require to access operating system

# Import pyserial library for working with USB/Serial ports
# https://pypi.org/project/pyserial
import serial, serial.tools.list_ports

# Refers to serial port
Serial_Port = None
SERIAL_PORT_NAME      = 'COM6' # Windows
SERIAL_PORT_BAUDRATE  = 9600

# Message configuration settings
HANDSHAKE = bytes([110]) # byte version of this ascii code
ZERO = bytes([0])
ONE = bytes([1])
MAX_MESSAGE_LENGTH = 256
MESSAGE = [bytes([0])] * MAX_MESSAGE_LENGTH
SLEEP_STANDARD = 4 # seconds to wait between interaction with devices

# Send a message
def SendMessage(thisPort):
  # https://stackoverflow.com/questions/34009653/convert-bytes-to-int
  # https://www.w3schools.com/python/python_for_loops.asp
  for i in range(0, int.from_bytes(MESSAGE[0], sys.byteorder) + 1): thisPort.write(MESSAGE[i])

# Exchange handshakes between the two devices
def DoHandshakes():
  # Wait for connection HANDSHAKE from the serial port.
  print("\nLooking for PORT_A handshake: ", end = '')
  MESSAGE[1] = ZERO
  while MESSAGE[1] != HANDSHAKE:
    MESSAGE[0] = ZERO
    while MESSAGE[0] != ONE: MESSAGE[0] = Serial_Port.read(1)
    MESSAGE[1] = Serial_Port.read(1)
  print("Success. Returning handshake to PORT_A.")
  SendMessage(Serial_Port)

# Creates the message queue
# https://docs.python.org/3.10/library/queue.html
messageQueue = queue.SimpleQueue() # first-in, first-out
messageQueue_mutex = threading.Lock() # necessary since messageQueue is accessed by independent processes

# Used as a signal to cause the thread to exit
USB_Serial_Connection_event = threading.Event()
USB_Serial_Connection_event.set()

# List all serial ports that have a device plugged in
# https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python
def FindSerialPorts():
  logging.info("Serial Ports:")
  Ports = serial.tools.list_ports.comports()
  found = False
  if Ports is not None:
    logging.info("Detected " + str(len(Ports)) + " total ports")
    for port in Ports:
      logging.info("\t" + str(port.name) + "\t:\t" + str(port.device) + "\t:\t" + str(port.manufacturer))
      found = True
  if not found: logging.info("No ports found")

# Connect to the correct serial port.
def ConnectSerialPort(GENERIC_PORT_NAME):
  global Serial_Port
  # Identify the device attached to the named serial port
  try:
    Serial_Port = serial.Serial(SERIAL_PORT_NAME, SERIAL_PORT_BAUDRATE)
  except Exception as thisException:  # https://docs.python.org/3/tutorial/errors.html
    logging.info(str(thisException))
    logging.info("\tIs the " + GENERIC_PORT_NAME + " device connected and active?")
    logging.info("\tHas " + GENERIC_PORT_NAME + " name and baudrate been correctly specified?")
    logging.info("\tIs the IDE Serial Monitor of the device connected to " + GENERIC_PORT_NAME + " been deactivated?")
    Serial_Port = None
    return
  time.sleep(SLEEP_STANDARD)  # wait long enough for the device to be ready
  # Get the "I am ready" message here, if there is to be one.
  # https://realpython.com/python-print/#preventing-line-breaks
  # https://stackoverflow.com/questions/14292746/how-to-python-convert-bytes-to-readable-ascii-unicode
  print("Connected to PORT_A.")
  print('\tPort Name:\t\t', Serial_Port.name)
  print('\tPort Baudrate:\t', Serial_Port.baudrate)

  # Wait for a handshake byte.
  #DoHandshakes()
  #print("Handshakes exchanged\n")

# Get the next message from the queue
def GetNextMessage():
  messageQueue_mutex.acquire()
  if messageQueue.empty():
    messageQueue_mutex.release()
    return None
  else:
    message = messageQueue.get()
    messageQueue_mutex.release()
    return message

# Thread for gathering messages as they arrive
def USB_Serial_Connection(name):
  global messageQueue, Serial_Port

  # Configure the log
  logFormat = "%(asctime)s: %(message)s"
  logging.basicConfig(format = logFormat, level = logging.INFO, datefmt = "%H:%M:%S")
  logging.info("%s: Starting", name)

  # Use this if uncertain as to which serial ports have devices plugged in
  FindSerialPorts()
  #exit(0)

  # Identify the device connected to the serial port
  print("\nConfiguring serial port " + SERIAL_PORT_NAME)
  ConnectSerialPort(SERIAL_PORT_NAME)
  if Serial_Port is None:
    logging.info("LoRa transceiver is not connected.")
    logging.info("Exiting thread. (" + name + ")" )
    exit(1)

  # Tell the LoRa device we are ready to receive and process messages.
  logging.info("\nAwaiting Messages...\n")
  #Serial_Port.flushInput() # https://stackoverflow.com/questions/7266558/pyserial-buffer-wont-flush
  # Receive messages and place in queue
  while USB_Serial_Connection_event.is_set():
    if Serial_Port.in_waiting > 0:
      messageLength = Serial_Port.read()
      incomingMessage = messageLength + Serial_Port.read(int.from_bytes(messageLength, sys.byteorder))
      messageQueue_mutex.acquire()
      messageQueue.put(incomingMessage)
      #print(incomingMessage)
      messageQueue_mutex.release()

  # Thread ends
  Serial_Port.close()
  Serial_Port = None
  logging.info("%s: Finished", name)
