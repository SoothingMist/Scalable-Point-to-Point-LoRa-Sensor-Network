
# https://coderslegacy.com/python/embed-matplotlib-graphs-in-tkinter-gui
# https://www.youtube.com/watch?v=lVTC8CvScQo
# https://stackoverflow.com/questions/7546050/switch-between-two-frames-in-tkinter
# https://www.geeksforgeeks.org/how-to-change-the-tkinter-label-text/
# https://www.delftstack.com/howto/python-tkinter/how-to-create-a-tkinter-window-with-a-constant-size
# https://www.geeksforgeeks.org/python-creating-a-button-in-tkinter
# https://www.plus2net.com/python/tkinter-place.php
# https://www.activestate.com/resources/quick-reads/how-to-position-buttons-in-tkinter-with-place
# https://github.com/turtlecode/Python-Tkinter-Animations-1/blob/main/animation.py
# https://medium.com/@rowangayleschaefer/an-excessively-gentle-guide-to-subplots-in-matplotlib-with-add-subplot-and-subplots-f14e6754a2e7
# https://pythonassets.com/posts/drop-down-list-combobox-in-tk-tkinter

# ================ Import External Libraries =============

# Import required matplotlib components
# https://matplotlib.org/stable/index.html
import matplotlib.pyplot as plt
from matplotlib import animation
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, NavigationToolbar2Tk)

# Import tkinter GUI library and components, integral to python language
# https://docs.python.org/3.10/library/tk.html
import tkinter
from tkinter import ttk # enables drop-down lists

# Import numpy library for math and matrices
# https://numpy.org
import numpy

# Import python native functions
import datetime # https://docs.python.org/3.10/library/datetime.html
import threading # https://docs.python.org/3.10/library/threading.html

# Import author's library for working with USB/Serial connections.
# Variables associated with the connection are set here.
import SerialUSB

# ========================================================

# ================ Create Basic GUI Frame ================

# Initialize Tkinter
root = tkinter.Tk()
root.title("Selected Camera and Single-Value Sensor")
root.geometry("1050x625+0+0") # https://www.tutorialspoint.com/how-to-specify-where-a-tkinter-window-should-open#:~:text=Tkinter%20window%20can%20be%20configured,open%20in%20a%20particular%20position.

# Initialize Matplotlib Subplots
plt.ion()
fig = plt.Figure()
fig.subplots_adjust(wspace=0.25) # https://stackoverflow.com/questions/6541123/improve-subplot-size-spacing-with-many-subplots

# Regarding the camera
cameraNomenclature_previous = ""
imageWidth = 500 # columns
imageHeight = 500 # rows
imageDepth = 3 # depth: RGB = 3, grayscale and black/white = 1, NRGB = 4
imageArray = numpy.zeros([imageHeight, imageWidth, imageDepth], dtype = numpy.uint8) # RGB cameras
camera = fig.add_subplot(121)
camera.axis('off')
camera.title.set_text\
  ("Camera " + str(imageWidth) + "x" + str(imageHeight) + "x" + str(imageDepth) + " max") # https://stackoverflow.com/questions/25239933/how-to-add-a-title-to-each-subplot
image = camera.imshow(imageArray)

# Regarding the line plot
graphNomenclature_previous = ""
MAX_SAMPLES = 10  # maximum number of samples appearing on the plot
x_values = []
y_values = []
yMin = -6.0
yMax = 6.0
graph = fig.add_subplot(122)
graph.title.set_text("Most-Recent Readings")
graph.set_xlim([0, MAX_SAMPLES])
graph.set_ylim([yMin, yMax])
for i in range(MAX_SAMPLES):
  x_values.append(i + 1)
  y_values.append(0)
line, = graph.plot(x_values, y_values, 'o')

# Create Tkinter Drawing Canvas that embeds matplotlib
# https://coderslegacy.com/figurecanvastkagg-matplotlib-tkinter
# https://www.activestate.com/resources/quick-reads/how-to-use-pack-in-tkinter
# https://stackoverflow.com/questions/58016467/how-to-make-tkinterpack-place-label-on-top-left-corner-in-below-program
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack \
  (side="top", anchor='nw', expand=False, fill="none")
canvas.draw()

# Add Toolbar
toolbar = NavigationToolbar2Tk(canvas, root, pack_toolbar=False)
toolbar.update()
toolbar.pack(side=tkinter.BOTTOM)

# Add camera's drop-down list
cameraLabel = tkinter.Label(text="Select Camera")
cameraLabel.place(relx = 0.17, rely = 0.88)
cameraSensors = []
cameraDropdown = ttk.Combobox(state="readonly", values=cameraSensors, width=25)
cameraDropdown.place(relx = 0.17, rely = 0.93)

# Add graph's drop-down list
graphLabel = tkinter.Label(text="Select Sensor")
graphLabel.place(relx = 0.17, rely = 0.76)
graphSensors = []
graphDropdown = ttk.Combobox(state="readonly", values=graphSensors, width=25)
graphDropdown.place(relx = 0.17, rely = 0.81)
graphAnnotation = tkinter.Label(text="Latest Sensor Reading")
graphAnnotation.place(relx = 0.55, rely = 0.76)
graphLatestReading = tkinter.StringVar()
graphLatestReading.set("<sensor's latest reading>")
graphMessage =\
  tkinter.Message(root, textvariable=graphLatestReading, justify=tkinter.LEFT, bg="white", width=400)
graphMessage.place(relx = 0.55, rely = 0.82)

# Add a general information display
maxGeneralInformationPostings = 14
generalLatestInformation = tkinter.StringVar()
generalLatestInformation_string = "<general information appears here>"
generalLatestInformation_string += "\n<displays " + str(maxGeneralInformationPostings) + " most recent messages>"
generalLatestInformation.set(generalLatestInformation_string)
generalInformation =\
  tkinter.Message(root, textvariable=generalLatestInformation, justify=tkinter.LEFT, bg="white", width=500)
generalInformation.place(x = 650, y = 0)

# Add new postings to the general information display
def postGeneralInformation(posting):
  global generalLatestInformation_string
  if generalLatestInformation_string.count('\n') < maxGeneralInformationPostings:
    generalLatestInformation_string += str('\n') + posting
  else:
    index = generalLatestInformation_string.find('\n')
    generalLatestInformation_string = \
      generalLatestInformation_string[index + 1 : len(generalLatestInformation_string)]
    generalLatestInformation_string += str('\n') + posting
  generalLatestInformation.set(generalLatestInformation_string)

# ========================================================

# ================ Global Variables ======================

# Constants are all in capital letters. Those values should not be changed.

# Data file to hold sensor data values as they arrive.
sensorDataFile = None
sensorDataFilename = "SensorData.csv"

MAX_MESSAGE_SIZE = 256 # LoRa message content has a maximum size

# Byte locations of header components within a message.
# See MessageContents.html for a detailed explanation.
LOCATION_MESSAGE_BYTES =   0
LOCATION_SYSTEM_ID =       1
LOCATION_SOURCE_ID =       2
LOCATION_DESTINATION_ID =  3
LOCATION_MESSAGE_ID =      4
LOCATION_MESSAGE_TYPE =    6
LOCATION_SENSOR_ID =       7
LOCATION_REBROADCASTS =    8
MESSAGE_HEADER_LENGTH =    9

# Communications thread
USB_Serial_Connection_thread = None

# ========================================================

# ================ Callable Functions ====================

# What happens when start_button is pressed
def Start_Button_functionality():
  # Start the thread that receives and queues messages
  global USB_Serial_Connection_thread, sensorDataFile, sensorDataFilename

  # This function runs only once
  # https://docs.python.org/3.10/library/threading.html
  if USB_Serial_Connection_thread is not None and \
     USB_Serial_Connection_thread.is_alive(): return

  # Create the thread
  USB_Serial_Connection_thread = threading.Thread\
    (target = SerialUSB.USB_Serial_Connection, args = ("USB_Serial_Connection",))

  # Start the thread
  USB_Serial_Connection_thread.start()

  # Open the file to which sensor data is written
  # https://learn.theprogrammingfoundation.org/programming/python/file-handling/?gclid=EAIaIQobChMIstuaj56_gQMVjt3jBx3n6AAeEAAYASAAEgLCSPD_BwE
  sensorDataFile = open(sensorDataFilename, "w")
  if sensorDataFile is None: print("Could not open the file ", sensorDataFilename)
  else: sensorDataFile.write("Date,Time,Node-Sensor-TypeData,Value Received\n")

# What happens when stop_button is pressed
def Stop_Button_functionality():
  global USB_Serial_Connection_thread, sensorDataFile

  if USB_Serial_Connection_thread is not None:
    SerialUSB.USB_Serial_Connection_event.clear()
    USB_Serial_Connection_thread.join()

  if sensorDataFile is not None:
    sensorDataFile.close()
    sensorDataFile = None

  root.quit()

# GUI animation function. Animates camera images and line graphs.
def animate(iteration):

  global graphNomenclature_previous, x_values, y_values, yMin, yMax#, sampleNumber
  global imageArray, cameraNomenclature_previous

  # Give a chance for other buttons to be checked
  root.update() # https://stackoverflow.com/questions/27050492/how-do-you-create-a-tkinter-gui-stop-button-to-break-an-infinite-loop

  # Retrieve next message
  message = SerialUSB.GetNextMessage()
  if message is not None:
      # Unpack the message
      messageID = UnpackMessage(message)

      if messageID is not None:
        print("Message ", messageID, " of type ", message[LOCATION_MESSAGE_TYPE], " (", len(message), ")", end = " ")

        # Check for message type 3, text-only notification
        if message[LOCATION_MESSAGE_TYPE] == 3:
          messageContents = message[MESSAGE_HEADER_LENGTH : len(message)]
          print('\t', messageContents)
          messageDecoded = messageContents.decode('ascii')
          postGeneralInformation(messageDecoded)
          messageDecoded = messageDecoded.split(':') # https://www.freecodecamp.org/news/how-to-parse-a-string-in-python

          # What to do with data
          if messageDecoded[0] == "DATA":

            # Find out what we need to know about the sensor producing the incoming data
            sensorNomenclature = str(message[LOCATION_SOURCE_ID]) + "-" + \
                                 str(message[LOCATION_SENSOR_ID]) + "-" + \
                                 messageDecoded[1]
            now = str(datetime.datetime.now())  # get the current date and time
            now = now[now.rfind(' ') + 1 : len(now)]  # format for this application
            now = now[0 : now.rfind('.')]
            if sensorDataFile is not None and messageDecoded[0].find("DATA") >= 0:  # https://www.w3schools.com/python/python_file_write.asp
              sensorDataFile.write(str(datetime.date.today()) + "," + now + "," +
                                   sensorNomenclature + "," + messageDecoded[2] + "\n")

            # See if we have that sensor already in our list.
            # If not already in the list, add it.
            # https://www.freecodecamp.org/news/python-find-in-list-how-to-find-the-index-of-an-item-or-element-in-a-list
            # https://stackoverflow.com/questions/51590357/appending-values-to-ttk-comboboxvalues-without-reloading-combobox
            if sensorNomenclature not in graphSensors:
              graphSensors.append(sensorNomenclature)
              graphDropdown['values'] = graphSensors
              if len(graphSensors) == 1:
                graphDropdown.set(graphSensors[0])
                graphNomenclature_previous = graphDropdown.get()

            # If a new sensor has been selected then reset the data vectors
            if graphNomenclature_previous != graphDropdown.get():
              y_values = []
              for index in range(MAX_SAMPLES): y_values.append(0)
              graphNomenclature_previous = graphDropdown.get()

            # Plot this data only if the sensor's nomenclature matches what has been selected
            if sensorNomenclature == graphDropdown.get():

              # https://www.tutorialspoint.com/how-to-rotate-tick-labels-in-a-subplot-in-matplotlib
              # https://www.geeksforgeeks.org/matplotlib-axes-axes-set_xticklabels-in-python
              graphLatestReading.set(messageDecoded[2] + " : " + now)
              currentY = float(messageDecoded[2])
              y_values.append(currentY)
              y_values = y_values[-MAX_SAMPLES:]

        # Check for message type 0, insert pixel data into image
        elif message[LOCATION_MESSAGE_TYPE] == 0:

          # Get the identity of the camera
          cameraNomenclature =\
            str(message[LOCATION_SOURCE_ID]) + "-" + str(message[LOCATION_SENSOR_ID])
          postGeneralInformation("Camera:" + cameraNomenclature)

          # See if we have that camera already in our list.
          # If not already in the list, add it.
          # https://www.freecodecamp.org/news/python-find-in-list-how-to-find-the-index-of-an-item-or-element-in-a-list
          # https://stackoverflow.com/questions/51590357/appending-values-to-ttk-comboboxvalues-without-reloading-combobox
          if cameraNomenclature not in cameraSensors:
            cameraSensors.append(cameraNomenclature)
            cameraDropdown['values'] = cameraSensors
            if len(cameraSensors) == 1:
              cameraDropdown.set(cameraSensors[0])

          # If a new camera has been selected then begin growing a new image matrix
          if cameraNomenclature_previous != cameraDropdown.get():
            imageArray = numpy.zeros([imageHeight, imageWidth, imageDepth], dtype=numpy.uint8)  # RGB cameras
            cameraNomenclature_previous = cameraDropdown.get()

          # Accept only if the camera's nomenclature matches what has been selected
          if cameraNomenclature == cameraDropdown.get():

            numPixels = message[MESSAGE_HEADER_LENGTH + 4]
            startRow =\
              (message[MESSAGE_HEADER_LENGTH] << 8) | message[MESSAGE_HEADER_LENGTH + 1]
            startColumn =\
              (message[MESSAGE_HEADER_LENGTH + 2] << 8) | message[MESSAGE_HEADER_LENGTH + 3]
            print("\t", startRow, " / ", startColumn)

            # Add new pixel data
            pixelDepth = message[MESSAGE_HEADER_LENGTH + 5]
            if startRow < imageHeight and \
               startColumn + numPixels < imageWidth and \
               pixelDepth <= imageDepth:

              messageByteIndex = MESSAGE_HEADER_LENGTH + 6
              for p in range(numPixels):
                # Get the pixel for the current column and put it in the image
                for d in range(pixelDepth):
                  imageArray[startRow, startColumn, d] = message[messageByteIndex]
                  messageByteIndex += 1
                startColumn += 1 # Get the next column
            else: print("\tAssumed image dimensions less than incoming image. Skipping.")

        # Message type not recognized
        else: print("\tMessage Type ", message[LOCATION_MESSAGE_TYPE], " not recognized. Message Rejected")

  line.set_data(x_values, y_values)
  image.set_data(imageArray)
  return image, line,

# Connect animate function to matplotlib animation facility
anim = animation.FuncAnimation(fig, animate, cache_frame_data=False, blit=True)

# Unpack a retrieved message
def UnpackMessage(thisMessage):

  if len(thisMessage) <= MESSAGE_HEADER_LENGTH:
    print("*** Rejecting too-short message")
    return None

  # Return the message ID
  return (thisMessage[LOCATION_MESSAGE_ID] << 8) | thisMessage[LOCATION_MESSAGE_ID + 1]

# ========================================================

# =============== Widgets Accessing Callable Functions ===

# Add the Start-Button
start_button =\
  tkinter.Button(root, text='Click to Start', bd='5',
                 command=Start_Button_functionality)
start_button.place(relx = 0.005, rely = 0.78)

# Add the Stop-Button
stop_button =\
  tkinter.Button(root, text='Click to Stop', bd='5',
                 command=Stop_Button_functionality)
stop_button.place(relx = 0.005, rely = 0.90)

# ========================================================

# Main process
if __name__ == '__main__':

  # Start the GUI
  print("\nPress START to begin")
  tkinter.mainloop() # Run the main application

  # Stop button pressed
  print("\nMain: Finished")
  exit(0)
