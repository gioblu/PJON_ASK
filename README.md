ASK_Slang 
====
ASK_Slang allows you to leverage all the power of simple ASK 433mhz transceivers and have long range digital data communication channel with a simple and user friendly set of functions to manage data transmission. ASK_Slang is based on [PJON](https://github.com/gioblu/PJON), contains a version of [digitalWriteFast](https://github.com/mpflaga/Arduino-digitalWriteFast) modified to work with arduino 1.0 version IDE and [Cape](https://github.com/gioblu/Cape) encryption library to optionally encrypt your data.

Range: 200 meters in city environment | Speed: 2857 baud/s - 357 bytes/s

## Why?
I am working with High Altitude Balloon probes and I need a reliable digital communication protocol I can fully understand that works good with cheap ASK 433Mhz radio transceivers ensuring long range accurate data stream.

* ASK_Slang works without timers or interrupts using only **micros()** and **delayMicroseconds()**
* contains a **packet manager**
* optional **collision avoidance** 
* optional **encryption** , based on [Cape](https://github.com/gioblu/Cape)
* optional **many to one communication** with 254 device assignable IDs.
