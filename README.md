# Scalable Peer-to-Peer LoRa Sensor Network
# Without LoRaWAN or Third-Party Services

Many applications require remote-sensing of data generated by various pieces of equipment. This project provides the latest code and documentation for an evolving proof-of-concept that applies LoRa point-to-point (peer-to-peer, P2P) radio broadcasts to link remote data to where it is needed. We do not use third-party services or LoRaWAN (although those are not inhibited if they are needed). Soil-moisture sensing in precision irrigation is the original target application but the approach described and demonstrated is general enough that other sensors could be employed. An example is multi-value sensors such as cameras. Open-Access software, hardware, and programming tools are used as much as possible. The basestation is hosted locally since it also is connected to a LoRa transceiver. Windows or Linux work well as the basestation’s OS. Associated hardware mainly involves RAKwireless equipment (https://rakwireless.kckb.st/b0a1be39). However, Arduino, Raspberry, and third-party sensors are also demonstrated as components which can be connected to RAKwireless baseboards.

Three papers have been published on this project:

 * https://academicjournals.org/journal/JECI/article-abstract/A973F1271109
 * http://informationanthology.net/OFE22-Raeth-OFE2023-paper.pdf
 * https://www.ispag.org/proceedings/?action=abstract&id=9795&title=LoRa+Flood-messaging+Sensor-data+Transport

A recorded presentation is also available: https://www.youtube.com/watch?v=VHDmGnvVWDE&t=12s

The papers and presentation refer to an earlier version of this project's equipment and software. A transition to new hardware was made necessary by the deprecation of the original transceiver. A suitable replacement had to be found. As the software transitioned to the new hardware, improvements were made. The project continues to evolve and grow.

Please post any comments under project issues. Always glad to hear them as they help the project improve.

# Project Overview

At issue with all forms of radio broadcast are inhibitions to propagation. For instance, there may be a hill between the transmitter and the receiver. As the number of sensors expands and the number of propagation inhibitors increases, a network of transceivers needs to be formed. This enhances the scalability of the overall system. That is the general purpose of LoRaWAN but that approach is for large-scale systems and can be expensive. Third-party services are also not used in this project because of their continuing costs and proprietary nature. This project's approach to building a network uses relatively simple and inexpensive point-to-point LoRa transceivers associated with a microcontroller board.

Inexpensive RAKwireless microcontroller development boards, coupled with their transceiver units, can be used to build a network of transceiver/sensing units in such a way that the overall system is scalable. In a sense, hardware is replaced by software. To be sure, the formal LoRaWAN standard has not been implemented. However, many of its attributes are present. To build a data-transport network of transceivers, this project uses flood-messaging. In many ways, flood-messaging is a lot like Ethernet's UDP networking. Delivery of messages is highly likely but not absolutely guaranteed. A rudimentary network with one sensor, relay, and basestation node can be constructed for less than $US100.

These are the originators of the concept of flood-messaging employed by this project:

   - Professor Philip Branch, School of Science, Computing and Engineering Technologies, Swinburne University of Technology, Melbourne, Australia
   - Professor Binghao Li, Faculty of Engineering, University of New South Wales, Sydney, Australia
   - Professor Kai Zhao, Faculty of Engineering, University of New South Wales, Sydney, Australia
  
Their paper discussed the design, implementation, and testing of their LoRa flood-messaging network. The citation and link to their paper is: Branch, P., Li, B., Zhao, K. (2020) A LoRa-Based Linear Sensor Network for Location Data in Underground Mining. MDPI, Telecom, 1(2), 68-79, https://www.mdpi.com/2673-4001/1/2/6.  

# License

GNU Affero General Public License v3.0

Permissions of this strongest copyleft license are conditioned on making available complete source code of licensed works and modifications, which include larger works using a licensed work, under the same license. Copyright and license notices must be preserved. Contributors provide an express grant of patent rights. When a modified version is used to provide a service over a network, the complete source code of the modified version must be made available.

This is not legal advice. Learn more about this license:
 * https://www.gnu.org/licenses/agpl-3.0.en.html
 * https://github.com/SoothingMist/Scalable-Point-to-Point-LoRa-Sensor-Network/blob/main/LICENSE

