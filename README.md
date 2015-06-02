ASK_Slang 
====

ASK_Slang is a device communications bus system that runs over ASK 433mhz radio transceivers, as VirtualWire or RadioHead creates a communication link between two distant arduino boards. This protocol is design to obtain a really high range and data communication accuracy. ASK_Slang works without the need of timers or interrupts using only **micros()** and **delayMicroseconds()** functions, contains a **packet manager**, optional **collision avoidance**, optional **encryption** and, optional **many to one communication** with 254 device assignable IDs.



 |Absolute bandwidth:  261 bytes/s    | Transfer speed: 357  bytes/s                
 |Practical bandwidth: 216 bytes/s    | Baud rate:      2857 baud/s                 
 |Accuracy: 96.45%                    | Range: 200 meters in city environment   
 
