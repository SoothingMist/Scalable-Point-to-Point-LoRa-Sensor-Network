
# https://realpython.com/intro-to-python-threading
# https://www.pythontutorial.net/python-concurrency/python-threading-lock

import logging # https://docs.python.org/3.10/library/logging.html
import threading # https://docs.python.org/3.10/library/threading.html
import time # https://docs.python.org/3.10/library/time.html
import queue # https://www.guru99.com/python-queue-example.html, https://docs.python.org/3.10/library/queue.html

# Import pyserial library for working with USB/Serial ports
# https://pypi.org/project/pyserial
import serial, serial.tools.list_ports

# Refers to serial port through which microcontroller talks to PC
Serial_Port = None
SERIAL_PORT_NAME = 'COM3' # Windows
SERIAL_PORT_BAUD_RATE = 9600

# Creates the message queue
# https://docs.python.org/3.10/library/queue.html
messageQueue = queue.SimpleQueue() # first-in, first-out
messageQueue_mutex = threading.Lock() # necessary since messageQueue is accessed by independent processes

# Used as a signal to cause the message-collection thread to exit
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
    Serial_Port = serial.Serial(SERIAL_PORT_NAME, SERIAL_PORT_BAUD_RATE)
  except Exception as thisException:  # https://docs.python.org/3/tutorial/errors.html
    logging.info(str(thisException))
    logging.info("\tIs the " + GENERIC_PORT_NAME + " device connected and active?")
    logging.info("\tHas " + GENERIC_PORT_NAME + " name and baudrate been correctly specified?")
    logging.info("\tIs the IDE Serial Monitor of the device connected to " + GENERIC_PORT_NAME + " been deactivated?")
    Serial_Port = None
    return
  time.sleep(5)  # wait long enough for the device to be ready

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

  # Identify the device connected to the serial port
  logging.info("Configuring serial port " + SERIAL_PORT_NAME)
  ConnectSerialPort(SERIAL_PORT_NAME)
  if Serial_Port is None:
    logging.info("LoRa transceiver is not connected.")
    logging.info("Exiting thread. (" + name + ")" )
    exit(1)

  # Tell the LoRa device we are ready to receive and process messages.
  logging.info("Awaiting Messages...\n")
  #Serial_Port.flushInput() # https://stackoverflow.com/questions/7266558/pyserial-buffer-wont-flush
  # Receive messages and place in queue
  while USB_Serial_Connection_event.is_set():
    if Serial_Port.in_waiting > 0:
      messageLength = Serial_Port.read()
      incomingMessage = messageLength + Serial_Port.read(int.from_bytes(messageLength, 'little') - 1)
      messageQueue_mutex.acquire()
      messageQueue.put(incomingMessage)
      messageQueue_mutex.release()

  # Thread ends
  Serial_Port.close()
  Serial_Port = None
  logging.info("%s: Finished", name)
