/*
 __  __      __     __      __
|__||__ |_/ |__ |  |__||\ || _
|  | __|| \  __||__|  || \||__| 0.1

Giovanni Blu Mitolo 2012 - 2015
gioscarab@gmail.com

ASK_Slang is a device communications bus system that runs over ASK 433mhz radio transceivers
and give to the user the chance to build a network of up to 254 Arduino boards wirelessly
connected on the same frequency. All is done without the need of timers or interrupts using
only micros() and delayMicroseconds() functions to achieve syncronization and bit timing.

ADDRESS: 255 different adresses can be assigned
CRC: XOR Cyclic Redundancy Check ensures almost errorless data communication
COLLISION DETECTION: collision avoidance is ensured analyzing network bus before starting
ENCRYPTION: Private key encryption + initialization vector to ensure almost random data stream
  __________________________________________________________________________________
 |ASK_Slang Standard mode | Tested on Arduino duemilanove - UNO - Arduino nano      |
 |----------------------------------------------------------------------------------|
 |BIT_WIDTH: 350 microseconds | BIT_SPACER: 750 microseconds                        |
 |----------------------------------------------------------------------------------|
 |Absolute bandwidth:  240 bytes/s    | Transfer speed: 357  bytes/s                |
 |Practical bandwidth: 216 bytes/s    | Baud rate:      2857 baud/s                 |
 |Accuracy: 96.45%                    |                                             |
 |----------------------------------------------------------------------------------| */

#ifndef ASK_Slang_h
  #define ASK_Slang_h

  #if defined(ARDUINO) && (ARDUINO >= 100)
    #include "Arduino.h"
    #include "includes/digitalWriteFast.h"
  #else
    #include "WProgram.h"
    #include "WConstants.h"
    #include "includes/digitalWriteFast.h"
  #endif
#endif

#define MAX_PACKET_LENGTH 20
#define BIT_WIDTH 350
#define BIT_SPACER 750

#define ACK  6
#define NAK  21
#define FAIL 0x100
#define BUSY 666
#define BROADCAST 124
#define TO_BE_SENT 74
#define CMD 88

#define ENCRYPTION_KEY "19id°?=(!$=<zkl"
#define ENCRYPTION_STRENGTH 2

#define SWAP(a,b) int t = _s_box[a]; _s_box[a] = _s_box[b]; _s_box[b] = t

#define MAX_PACKETS 20

struct packet {
  uint8_t device_id;
  char *content;
  int state;
  unsigned long registration;
  unsigned long timing;
};

class ASK_Slang {

  public:

    ASK_Slang(int input_pin, uint8_t ID);
    void set_collision_avoidance(boolean state);
    void set_encryption(boolean state);

    void update();
    int send(uint8_t ID, char *packet, unsigned long timing = 0);
    void remove(int packet_id);

    void crypt(char *content, boolean initialization_vector = false, boolean side = false);
    uint8_t generate_IV(uint8_t string_length);

    void send_bit(uint8_t VALUE, int duration);
    void send_byte(uint8_t b);

    int send_string(uint8_t ID, char *string);
    int send_string(uint8_t ID, char *string, int count);

    int send_command(uint8_t ID, uint8_t command_type, unsigned int value, unsigned long timing = 0);
    int send_command(uint8_t ID, uint8_t command_type, uint8_t value, unsigned long timing = 0);

    boolean can_start();
    uint8_t read_byte();
    int receive_byte();
    int receive();
    int receive(unsigned long duration);

    uint8_t data[MAX_PACKET_LENGTH];
    char hash[MAX_PACKET_LENGTH];
    packet packets[MAX_PACKETS];

  private:

    uint8_t _device_id;
    int _input_pin;
    int _READ_DELAY;
    unsigned char _s_box[ENCRYPTION_STRENGTH];

    boolean _collision_avoidance;
    boolean _encryption;
};
