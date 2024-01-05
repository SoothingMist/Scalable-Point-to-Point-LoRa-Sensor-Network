# Scalable Peer-to-Peer LoRa Sensor Network
# Without LoRaWAN or Third-Party Services

Many applications, including precision irrigation, require remote-sensing of data generated by various pieces of equipment. This page provides the currently-existing code and documentation for an evolving proof-of-concept that applies LoRa point-to-point (peer-to-peer) radio broadcasts to link remote data to where it is needed. We do not use third-party services or LoRaWAN. Soil-moisture is the original application but the approach is general enough that other sensors could be employed. An example is multi-value sensors such as cameras. Open-Access software, hardware, and programming tools are used as much as possible. The basestation can be hosted locally since it also is connected to a LoRa transceiver. Windows or Linux works well as the basestation’s OS. Hardware involves PC, Arduino, Raspberry, and Dragino equipment.

*** IMPORTANT: This project uses Dragino’s Lora Shield. Dragino’s Lora Shield is NOT Dragino’s LA66 LoRaWAN shield. That is something different. The software developed in this project will not run on that shield. At the start of 2024 I found out that Dragino’s Lora Shield is at its end-of-life, is scheduled for obsolescence, and will be discontinued. It is still available through many sellers. Their new shield (LA66 LoRaWAN), according to its documentation, is capable of both peer-to-peer and LoRaWAN. Message handling in this project's software is kept seperate from code that interacts with LoRa. Thus, transition should be achievable, although the new shield uses a completely different software library.

Two papers have been published on this project:

 * https://academicjournals.org/journal/JECI/article-abstract/A973F1271109
 * http://informationanthology.net/OFE22-Raeth-OFE2023-paper.pdf

Please post any comments under project issues. Glad to hear them as they help the project improve.

# Project Overview

At issue with all forms of radio broadcast are inhibitions to propagation. For instance, there may be a hill between the transmitter and the receiver. As the number of sensors expands and the number of propagation inhibitors increases, a network of transceivers needs to be formed. This enhances the scalability of the overall system. That is the general purpose of LoRaWAN but that approach can be expensive and complicated. Third-party services are also not used because of their continuing costs and proprietary nature. Our approach to building a network uses simple and relatively inexpensive point-to-point LoRa transceivers plugged into a microcontroller board.

This project uses ArduinoUnoR3/Dragino microcontroller/transceiver units to build a network of transceiver/sensing units in such a way that the overall system is scalable. In a sense, hardware is replaced by software. To be sure, the formal LoRaWAN standard has not been implemented. However, many of its attributes are present. Where sensors that deliver more than one value are concerned, dealing with such sensors may require more memory than the Uno R3 can provide. There is also the issue of the sensor not being compatible with the transceiver because of the way the two integrate with the microcontroller board. In such a case, a Raspberry headless computer is employed to create a programmable USB/Serial hub to which two Uno R3 microcontrollers are connected, one for the sensor and one for the transceiver. I2C communication is not used because of the size and volume of messages.

Flood-Messaging is the approach used in this project. In many ways, it is a lot like UDP networking. Delivery of messages is highly likely but not absolutely guaranteed. A full paper has been submitted for publication in the peer-reviewed literature on the underlying LoRa flood-messaging proof of concept that underlies this project. These are the contributors to that phase of the project:

- Professor Philip Branch, School of Science, Computing and Engineering Technologies, Swinburne University of Technology, Melbourne, Australia
- Professor Binghao Li, Faculty of Engineering, University of New South Wales, Sydney, Australia
- Professor Kai Zhao, Faculty of Engineering, University of New South Wales, Sydney, Australia
  
Their earlier paper discussed the design, implementation, and testing of their LoRa flood-messaging network. The citation and link to their paper is: Branch, P., Li, B., Zhao, K. (2020) A LoRa-Based Linear Sensor Network for Location Data in Underground Mining. MDPI, Telecom, 1(2), 68-79, https://www.mdpi.com/2673-4001/1/2/6.

# Files

Each module listed below contains software and documentation in a zip file for a project component. These modules represent a phase of the project and are meant to operate without any of the other modules.

* EncryptDecrypt: Demonstrates how to add AES encryption/decryption to ArduinoUno/Dragino microcontroller/transceiver units.

* Uno-On-off: Demonstrates Arduino's external on/off timer as a means of saving battery charge.

* Remote-Soil-Moisture-Sensing: A first shot at using LoRa for remote sensing.

* LoRaFloodMessaging: Software and background for the LoRa Flood Messaging proof of concept.

* CaptureCameraFrames_Uploaded: Demonstration of Arduino Potenta H7 microcontroller board coupled with Vision Shield - LoRa to capture, send via USB/Serial, and display the camera's image on a PC. This is part of a learning process aimed at eventual LoRa broadcast of images.

* CaptureAndSendFrames_Uploaded: Functionally similar to CaptureCameraFrames_Uploaded but uses LoRa-sized messages that contain a CRC check value.

* Programmable_USB_Hub: The H7 device did not work out for this effort since its camera is only grayscale and it will not do LoRa point-to-point, as far as I can determine. This code demonstrates the basic idea of linking two Arduino Uno microcontrollers via a programmable USB hub.

* Pixy-ImageSegmentation: Demonstrates a microcontroller/camera unit sending LoRa-sized image segments to a PC. The segments are reconstructed and displayed. LoRa itself is not yet engaged.

* USB_Hub_LoRa_Images: Demonstrates an application of programmable USB/Serial hubs. Two disparate devices are enabled to communicate. The application captures camera data from one device and broadcasts it via LoRa point-to-point on another device. A basestation receives the broadcasts and reconstructs the image.

* LoRaBinaryFloodMessaging: Binary-Messages version of LoRaFloodMessaging. Includes single-value sensor and camera. Evolving project. Binary messages and flood-messaging are fully implemented. Documentation explains setup and operation.

# License

GNU Affero General Public License v3.0

Permissions of this strongest copyleft license are conditioned on making available complete source code of licensed works and modifications, which include larger works using a licensed work, under the same license. Copyright and license notices must be preserved. Contributors provide an express grant of patent rights. When a modified version is used to provide a service over a network, the complete source code of the modified version must be made available.

This is not legal advice. Learn more about repository licenses: https://docs.github.com/articles/licensing-a-repository/#disclaimer.
Additional license information is here: https://github.com/SoothingMist/Remote-Soil-Moisture-Sensing/blob/main/LICENSE.
