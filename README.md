PJON_ASK v1.0 stable
==== 
 
Arduino compatible implementation of PJON for ASK 433Mhz radio modules:
![alt tag](http://img.banggood.com/images/upload/2012/chenjianwei/SKU064487.5.jpg)

PJON (Padded Jittering Operative Network) is an opensource multi-master communication bus system standard. Its 433Mhz radio implementation, PJON_ASK, can be a valid alternative to VirtualWire library (generally used for data transmission with this modules). With PJON_ASK you have a real wireless network with up to 255 indexed devices communicating through packets in half-duplex or simplex mode with CRC error detection and correct reception acknowledge. If you are interested to know more about the PJON standard, visit the [wiki](https://github.com/gioblu/PJON/wiki). 

```cpp  
#include <PJON_ASK.h>     // Transmitter board code
PJON network(11, 12, 45); // receiver to pin 11, transmitter to pin 12, device id 45

void setup() {
  network.send(44, "B", 1, 1000000); 
  // Send to device 44, "B" content of 1 byte length every 1000000 microseconds (1 second)
}

void loop() {
  network.update();
}

/* ---------------------------------------------------------------------------- */

#include <PJON_ASK.h>     // Receiver board code
PJON network(11, 12, 44); // receiver to pin 11, transmitter to pin 12, device id 44

void setup() {
  network.set_receiver(receiver_function); // Set the function used to receive messages
};

static void receiver_function(uint8_t length, uint8_t *payload) {
  if(payload[0] == 'B') { // If the first letter of the received message is B 
    digitalWrite(13, HIGH);
    delay(30);
    digitalWrite(13, LOW);
  }
}

void loop() {
  network.receive(1000);
}
```

####Features
- ASK 315/433Mhz cheap transceiver implementation (200m range in urban environment) 
- Works with both simplex and half duplex setup. 
- Device id implementation to enable univocal communication up to 254 devices.  
- Cyclic Redundancy Check (CRC).
- Acknowledgement of correct packet sending. 
- Collision avoidance to enable multi-master capability.
- Broadcast functionality to contact all connected devices.
- Packet manager to track and retransmit failed packet sendings in background.
- Error handling.

####Performance
- Transfer speed: **256 B/s** or **2564 Baud** 
- Data throughput: **216 B/s** 
- Accuracy: **99.995%**
- Range: **200 meters** in urban environment (still not tested with LOS in open air)

####Compatibility
- Arduino Diecimila / Duemilanove
- Arduino Mini
- Arduino Uno
- Arduino Nano
- Arduino Mega

This library is tested effectively with many versions of the ASK 433Mhz module present on ebay and various other webshops.
This implementation works with a simple oversampling approach, for this reason works also on wires and other radio modules. If you need communication through wire check the PJON wire implementation [here](https://github.com/gioblu/PJON).

####Why not VirtualWire?
I don't think VirtualWire is clear, efficient and understandable enough to be the standard library for wireless radio communication available to the community, because of its implementation mess and complexity. Moreover VirtualWire doesn't have the support for multiple devices in multimaster setup, CRC, acknowledge, collision avoidance and packet management. For this reason I wrote this implementation to provide the user with the PJON standard on wireless. :)


## How to start
The first step is the physical layer. The suggested antenna if you have space available is a 69cm dipole antenna for both transmitter and receiver module:
```cpp  
       
        345mm                    345mm                  345mm                    345mm
   -------------------|--------------------        -------------------|--------------------
                    __|__                                          ___|________
                   | tx  |                                        | rx         |
                   |_____|                                        |____________|
                       
```

Lets start coding, instantiate the `PJON` object that in the example is called network. To initialize a network based on PJON you need only to define the reception and transmission pin (any free digital pin on your board) where ASK receiver and transmitter are connected and a unique ID (0 - 255):

```cpp  
  PJON network(11, 12, 123); 
```

If you have only the transmitter on a board and the receiver on anotherone, you transmit in simplex mode. You should pass  `NOT_USED` instead of the receiver pin:
```cpp  
  PJON network(NOT_USED, 12, 123); 
```

On the other side if you have only the receiver module:
```cpp  
  PJON network(12, NOT_USED, 123); 
```

Take in consideration that in simplex mode is impossible to know if the receiver got the right message. This happens because you don't have on the transmitter side a receiver module able to hear the `ACK`, for this reason if one of the pins are `NOT_USED` PJON_ASK runs in simplex mode not sending `ACK` and not checking if the channel is used.


## Transmit data
Data transmission is handled by a packet manager, the `update()` function has to be called at least once per loop cycle. Consider that this is not an interrupt driven system, all the time dedicated to delays or executing other tasks is postponing the sending of all the packets are scheduled to be sent:

```cpp  
  network.update(); 
```

To send a string to another device connected to the bus simply call `send()` function passing the ID you want to contact, the string you want to send and its length:

```cpp
network.send(100, "Ciao, this is a test!", 21);
```
Packet length is there to prevent unwanted buffer overflows. If sending arbitrary values  `NULL` terminator strategy (strlen) is not safe to detect the end of a string. 

To send a value repeatedly simply add as last parameter the interval in microseconds you want between every sending:

```cpp
int one_second_delay_test = network.send(100, "Test sent every second!", 23, 1000000);
```

`one_second_delay_test` contains the id of the packet. If you want to remove this repeated task simply:

```cpp
network.remove(one_second_delay_test);
```

To broadcast a message to all connected devices, use the `BROADCAST` constant as the recipient ID. Every node will receive the message but will not answer `ACK` to avoid communication overlap.

```cpp
int broadcastTest = network.send(BROADCAST, "Message for all connected devices.", 34);
```

## Receive data
Now define a `static void function` that will be called if a correct message is received. This function receives 2 parameters: the message length and its content.

```cpp
static void receiver_function(uint8_t length, uint8_t *payload) {
  Serial.print("Message content: ");

  for(int i = 0; i < length; i++) 
    Serial.print((char)payload[i]);
  
  Serial.print(" | Message length: ");
  Serial.println(length);
};
```

Inform the network to call `receiver_function` when a correct message is received:

```cpp
network.set_receiver(receiver_function);
```

To correctly receive data call `receive()` function at least once per loop cycle passing as a parameter, the maximum reception time in microseconds:
```cpp
int response = network.receive(1000);
```

Consider that this is not an interrupt driven system and so all the time passed in delay or executing something a certain amount of packets will be potentially lost unheard. Structure intelligently your loop cycle to avoid huge blind timeframes.



##Error handling
PJON is designed to inform the user if the communication link to a certain device is lost or if the packtes buffer is full. A `static void function` has to be defined as the error handler, it receives 2 parameters the first is the error code and the second is 1 byte additional data related to the error.

Error types:
- `CONNECTION_LOST` (value 101), `data` parameter contains lost device's id.
- `PACKETS_BUFFER_FULL` (value 102), `data` parameter contains buffer length.
- `MEMORY_FULL` (103), `data` parameter contains `FAIL`.


```cpp
static void error_handler(uint8_t code, uint8_t data) {
  if(code == CONNECTION_LOST) {
    Serial.print("Connection with device ID ");
    Serial.print(data);
    Serial.println(" is lost.");
  }
  if(code == PACKETS_BUFFER_FULL) {
    Serial.print("Packet buffer is full, has now a length of ");
    Serial.println(data);
    Serial.println("Possible wrong network configuration!");
    Serial.println("For high complexity networks higher MAX_PACKETS over 10.");
    Serial.println("See in PJON.h");
  }
  if(code == MEMORY_FULL) {
    Serial.println("Packet memory allocation failed. Memory is full.");
  }
}
```


Now inform the network to call the error handler function in case of error:
```cpp
network.set_error(error_handler);
```
---

Copyright (c) 2012-2015, Giovanni Blu Mitolo
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   This product includes software developed by the <organization>.
4. Neither the name of the <organization> nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the copyright holder or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

