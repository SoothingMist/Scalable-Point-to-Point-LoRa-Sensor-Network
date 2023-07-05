# Scalable ArduinoUno/Dragino LoRa Sensor Network
# - Without LoRaWAN

In an earlier effort, https://github.com/SoothingMist/Remote-Soil-Moisture-Sensing, a prototype for using LoRa radio broadcasts to remotely monitor sensors was demonstrated. Soil-moisture is the application but the approach is general enough that other sensors could be employed. Open-Access software and programming tools were used exclusively. Open-Access hardware was employed as much as possible. The basestation can be hosted locally, with no network employed, or remotely, via the internet or a local network.

At issue with the earlier effort is that it works well in situations where there are no inhibitions to the propagation of the transmitter’s radio broadcasts. For instance, there may be a hill between the transmitter and the receiver. As the number of sensors expands and the number of propagation inhibitors increases, a network of transceivers would need to be formed. This would enhance the scalability of the overall system.

This present project uses ArduinoUno/Dragino microcontroller/transceiver units from the earlier effort to build a network of transceiver/sensing units in such a way that the overall system is scalable but without using complex and expensive LoRaWAN. In a sense, hardware is replaced by software. To be sure, the formal LoRaWAN standard has not been implemented. However, many of its attributes are present.

Each zip file contains a component of the system. Be sure to see ReadMe.pdf within each zip file for details.

A full paper has been submitted for publication in the peer-reviewed literature on the LoRa flood-messaging proof of concept. 

These are contributors to that project.

- Professor Philip Branch, School of Science, Computing and Engineering Technologies, Swinburne University of Technology,
Melbourne, Australia

- Professor Binghao Li, Faculty of Engineering, University of New South Wales, Sydney, Australia

- Professor Kai Zhao, Faculty of Engineering, University of New South Wales, Sydney, Australia
  
Their earlier paper discussed the design, implementation, and testing of their flood-messaging network. The citation and link to their paper is: Branch, P., Li, B., Zhao, K. (2020) A LoRa-Based Linear Sensor Network for Location Data in Underground Mining. MDPI, Telecom, 1(2), 68-79, https://www.mdpi.com/2673-4001/1/2/6.


# Files

* EncryptDecrypt: Demonstrates how to add AES encryption/decryption to ArduinoUno/Dragino microcontroller/transceiver units.

* Uno-On-off: Demonstrates Arduino's external on/off timer as a means of saving battery charge.

* LoRaFloodMessaging: Software and background for the LoRa Flood Messaging proof of concept.

* CaptureCameraFrames_Uploaded: Demonstration of Arduino Potenta H7 microcontroller board coupled with Vision Shield - LoRa to capture, send via USB/Serial, and display the camera's image on a PC. This is part of a learning process aimed at eventual LoRa broadcast of images.

* CaptureAndSendFrames_Uploaded: Functionally similar to CaptureCameraFrames_Uploaded but uses LoRa-sized messages that contain a CRC check value.

* Programmable_USB_Hub: The H7 device did not work out for this effort since its camera is only grayscale and it will not do LoRa point-to-point, as far as I can determine. Going to try linking two Arduino Uno microcontrollers via a programmable USB hub. This code demonstrates the basic concept.

# License

GNU Affero General Public License v3.0

Permissions of this strongest copyleft license are conditioned on making available complete source code of licensed works and modifications, which include larger works using a licensed work, under the same license. Copyright and license notices must be preserved. Contributors provide an express grant of patent rights. When a modified version is used to provide a service over a network, the complete source code of the modified version must be made available.

This is not legal advice. Learn more about repository licenses: https://docs.github.com/articles/licensing-a-repository/#disclaimer.
Additional license information is here: https://github.com/SoothingMist/Remote-Soil-Moisture-Sensing/blob/main/LICENSE.
