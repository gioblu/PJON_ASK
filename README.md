ASK_Slang 
====

ASK_Slang is a device communications bus system that runs over ASK 433mhz radio transceivers, as VirtualWire or RadioHead creates a communication link between two distant arduino boards. ASK_Slang works without the need of timers or interrupts using only micros() and delayMicroseconds() functions to achieve syncronization and bit timing.

Using a simple ASK 433Mhz TX/RX pair setup and VirtualWire / RadioHead code examples the range and data reception accuracy are not impressive. My results: Range: 60m in city enviroment with 64% of communication accuracy.
I am writing ASK_Slang because I need a long range and reliable communication link based on ASK 433Mhz TX/RX pair.
The actual results are for sure promising: 

 |Absolute bandwidth:  261 bytes/s    | Transfer speed: 357  bytes/s                
 |Practical bandwidth: 216 bytes/s    | Baud rate:      2857 baud/s                 
 |Accuracy: 96.45%                    | Range: 200 meters in city environment   
 
