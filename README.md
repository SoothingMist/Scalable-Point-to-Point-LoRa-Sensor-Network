# Scalable Arduino-Based LoRa Sensor Network
Uses Arduino-Uno to build a network of LoRa transceivers.


In an earlier effort, https://github.com/SoothingMist/Remote-Soil-Moisture-Sensing, a prototype for using LoRa radio broadcasts to remotely monitor sensors was demonstrated. Soil-moisture is the application but the approach is general enough that other sensors could be employed. Open-Source software and hardware were employed as much as possible. The basestation can be hosted locally, with no network employed, or remotely, via the internet or a local network.

At issue with the earlier effort is that it works well in situations where there are no inhibitions to the propagation of the transmitter’s radio broadcasts. For instance, there may be a hill between the transmitter and the receiver. As the number of sensors expands and the number of propagation inhibitors increases, a network of transceivers would need to be formed. This would enhance the scalability of the overall system.

This present project seeks to use transceiver units from the earlier effort to build a network of transceiver/sensing units in such a way that the overall system is scalable but without using formal and expensive LoRaWAN equipment. In a sense, hardware will be replaced by software. To be sure, the formal LoRaWAN standard will not be implemented. However, many of its attributes will be present.

This particular posting demonstrates AES encryption/decryption as signals pass from transmitter to receiver.

Be sure to see ReadMe.pdf within this project's zip file for more details.
