ASK_Slang 
====
ASK_Slang allows you to leverage all the power of simple ASK 433mhz transceivers and have long range digital data communication channel with a simple and user friendly set of functions to manage data transmission. ASK_Slang is based on [PJON](https://github.com/gioblu/PJON), contains a version of [digitalWriteFast](https://github.com/mpflaga/Arduino-digitalWriteFast) modified to work with arduino 1.0 version IDE and [Cape](https://github.com/gioblu/Cape) encryption library to optionally encrypt your data.

Range: 200 meters in city environment | Speed: 2857 baud/s - 357 bytes/s

## Why?
I am working with High Altitude Balloon probes and I need a reliable digital communication protocol I can fully understand that works good with cheap ASK 433Mhz radio transceivers ensuring long range accurate data stream.

## What?

* ASK_Slang works without timers or interrupts using only **micros()** and **delayMicroseconds()**
* 254 selectable IDs to be assigned to your devices:
* 
  ```cpp  
  ASK_Slang network(/*Radio I/O Pin*/ 12, /*Selecetd device ID*/ 133);
  ```
  
* optional **encryption** based on [Cape](https://github.com/gioblu/Cape)
  ```cpp
  network.set_encryption(true); 
  ```
* **send a string** to a certain device:
  ```cpp
  network.send(/*Recipient ID*/ 33, /*Content*/ "Ciao!"); 
  network.update(); // Call update() at least once every loop 
  ```
  
* use the **packet manager** and schedule a sending:
  ```cpp
  network.send(/*Recipient ID*/ 33, /*Content*/ "Ciao!", /*Interval*/ 1000); 
  network.update(); // Call update() at least once every loop 
  ```


