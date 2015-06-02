/*
 __  __      __     __      __
|__||__ |_/ |__ |  |__||\ || _
|  | __|| \  __||__|  || \||__| 0.1

Giovanni Blu Mitolo 2012 - 2015
gioscarab@gmail.com

ASK_Slang is a device communications bus system that runs over ASK 433mhz radio transceivers
and gives to the user the chance to build a network of up to 255 Arduino boards wirelessly
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
 |Absolute bandwidth:  261 bytes/s    | Transfer speed: 357  bytes/s                |
 |Practical bandwidth: 216 bytes/s    | Baud rate:      2857 baud/s                 |
 |Accuracy: 96.45%                    | Range: 200 meters in city environment       |
 |----------------------------------------------------------------------------------| */

#include "ASK_Slang.h"

/* Initiate ASK_Slang passing pin number and the selected device_id
   Avoid 124 device_id because is used as BROADCAST symbol */

ASK_Slang::ASK_Slang(int input_pin, uint8_t device_id) {
  _input_pin = input_pin;
  _device_id = device_id;

  for(int i = 0; i < MAX_PACKETS; i++) {
    packets[i].state = NULL;
    packets[i].timing = 0;
  }
}


/* Set collision detection:
 If true Avoids to transmit over other transmitting devices:
 on top of this can be developed a multimaster network of Arduino boards
 that arbitrally decides to communicate (slight reduction of bandwidth) */

void ASK_Slang::set_collision_avoidance(boolean state) {
  _collision_avoidance = state;
}


/* Set Encryption state to transmitted data */

void ASK_Slang::set_encryption(boolean state) {
  _encryption = state;
}


/* Check if the channel if free for transmission:
 If an entire byte received contains no 1s it means
 that there is no active transmission */

boolean ASK_Slang::can_start() {
  pinModeFast(_input_pin, INPUT);
  this->send_bit(0, 2);
  if(!this->read_byte())
    return true;

  return false;
}


/* Encrypt string with a custom made private key algorithm + initialization vector */

void ASK_Slang::crypt(char *content, boolean initialization_vector, boolean side) {
  uint8_t i, j = 0;
  uint8_t string_length = strlen(content);
  uint8_t encryption_key_length = strlen(ENCRYPTION_KEY);

  if(initialization_vector && side)
    for(i = 0; i < string_length; i++)
      content[i] ^= content[string_length - 1];

  for (i = 0; i < ENCRYPTION_STRENGTH; i++)
    _s_box[i] = i;
  
  for (i = 0; i < ENCRYPTION_STRENGTH; i++) {
    j = (j + _s_box[i] + ENCRYPTION_KEY[i % encryption_key_length]) % ENCRYPTION_STRENGTH;
    SWAP(_s_box[i], _s_box[j]);
  }

  i = j = 0;
  for (int k = 0; k < string_length; k++) {
    i = (i + 1) % ENCRYPTION_STRENGTH;
    j = (j + _s_box[i]) % ENCRYPTION_STRENGTH;
    SWAP(_s_box[i], _s_box[j]);
    hash[k] = content[k] ^ _s_box[ (_s_box[i] + _s_box[j]) % ENCRYPTION_STRENGTH];
  }

  if(initialization_vector && !side) {
    hash[string_length] = this->generate_IV(string_length);
    for(i = 0; i < string_length; i++)
      hash[i] ^= hash[string_length];
  }

  hash[string_length + 1] = '\0';
}

uint8_t ASK_Slang::generate_IV(uint8_t string_length) {
  uint8_t IV = (micros() % 254) + 1;
  for(uint8_t i = 0; i < string_length; i++)
    if(hash[i] == IV)
      return this->generate_IV(string_length);
  return IV;
}


/* Transmitter side functions --------------------------------------------------------------------------- */

/* Send a bit to the pin
 digitalWriteFast is used instead of standard digitalWrite
 function to optimize transmission time */

void ASK_Slang::send_bit(uint8_t VALUE, int duration) {
  digitalWriteFast(_input_pin, VALUE);
  delayMicroseconds(duration);
}


/* Send a byte to the pin:
  ________ _______________________
 |  Init  |         Byte          |
 |--------|-----------------------|
 |_____   |__       __    __ __   |
 |     |  |  |     |  |  |     |  |
 |1    |0 |1 |0  0 |1 |0 |1  1 |0 |
 |_____|__|__|__ __|__|__|_____|__|

 Init is a long 1 with a BIT_SPACER duration and a standard bit 0
 after that comes before the raw byte */

void ASK_Slang::send_byte(uint8_t b) {
  digitalWriteFast(_input_pin, HIGH);
  delayMicroseconds(BIT_SPACER);
  digitalWriteFast(_input_pin, LOW);
  delayMicroseconds(BIT_WIDTH);

  for(uint8_t mask = 0x01; mask; mask <<= 1)
    this->send_bit(b & mask, BIT_WIDTH);
}


/* Send a string to the pin:
 An Example of how the string "HI" is formatted and sent:
  _____    _________________________________________ 
 | C-A |  | ID | LENGTH | byte0 | byte1  | IV | CRC |
 |-----|->|----|--------|-------|--------|----|-----|
 |  0  |  | 12 |   6    |   H   |   I    | 43 | 134 |
 |_____|  |____|________|_______|________|____|_____|
  
 If you are calling cyclically send_string you will need at least 200 microseconds
 between every function call to permit correct pairing of the receiver
 
 C-A: Collision avoidance - receive a byte, if no 1s channel is free    - 1 byte
 ID: Receiver ID                                                        - 1 byte
 LENGTH: Length of the string (max 255 characters)                      - 1 byte
 IV: Initialization vector, present if encryption activated             - 1 byte
 CRC: Cyclic redundancy check                                           - 1 byte */

int ASK_Slang::send_string(uint8_t ID, char *string) {

  uint8_t package_length = strlen(string) + 4;
  uint8_t CRC = 0;

  if(_collision_avoidance && !this->can_start())
      return BUSY;

  if(_encryption)
    this->crypt(string, true, 0);

  pinModeFast(_input_pin, OUTPUT);
  this->send_byte(ID);
  CRC ^= ID;
  this->send_byte(package_length);
  CRC ^= package_length;

  char *string_pointer = (_encryption) ? hash : string;
  while(*string_pointer) {
    this->send_byte(*string_pointer);
    CRC ^= *string_pointer;
    *string_pointer++;
  }

  this->send_byte(CRC);
  digitalWriteFast(_input_pin, LOW);

  return ACK;
};

int ASK_Slang::send_string(uint8_t ID, char *string, int count) {
  int response;
  for(int i = 0; i < count && response != ACK; i++)
    response = this->send_string(ID, string);

  return response;
}


/* Send a command to the pin:
 An Example of how a command is formatted and sent:
  _____    __________________________________________________________ 
 | C-A |  |  ID   | LENGTH |  CMD  |  type  |  value |  IV   |  CRC  |
 |-----|->|-------|--------|-------|--------|--------|-------|-------|
 |  0  |  | 0-255 |   7    |   88  |  0-255 |  0-255 | 0-255 | 0-255 |
 |_____|  |_______|________|_______|________|________|_______|_______|

 C-A: Collision avoidance - receive a byte, if no 1s channel is free           - 1 byte
 ID: Receiver ID                                                               - 1 byte
 LENGTH: Length of the string (max 255 characters)                             - 1 byte

 CMD: 88 is the command symbol                                                 - 1 byte
 type: 0-255 byte value that has to be equal to its related reaction's id      - 1 byte
 value: 1 or 2 bytes dedicated to command additional data                      - 1 byte

 IV: Initialization vector, present if encryption activated                    - 1 byte
 CRC: Cyclic redundancy check                                                  - 1 byte */

int ASK_Slang::send_command(byte ID, byte command_type, uint8_t value, unsigned long timing) {
  char bytes_to_send[3] = { CMD, command_type, value };
  return this->send(ID, bytes_to_send, timing);
}

int ASK_Slang::send_command(byte ID, byte command_type, unsigned int value, unsigned long timing) {
  char bytes_to_send[4] = { CMD, command_type, value >> 8, value & 0xFF };
  return this->send(ID, bytes_to_send, timing);
}


/* Insert a packet in the send list:
 The added packet will be sent in the next update() call. 
 Using the variable timing is possible to set the delay between every 
 transmission cyclically sending the packet (use remove() function stop it) */ 

int ASK_Slang::send(uint8_t ID, char *packet, unsigned long timing) {
  for(uint8_t i = 0; i < MAX_PACKETS; i++)
    if(packets[i].state == NULL) {
      packets[i].state = TO_BE_SENT;
      packets[i].content = packet;
      packets[i].device_id = ID;
      if(timing > 0) {
        packets[i].registration = micros();
        packets[i].timing = timing;
      }
      return i;
    }

  return -1;
}


/* Update the state of the send list:
 Check if there are packets to send, erase the correctly delivered */ 

void ASK_Slang::update() {
  for(uint8_t i = 0; i < MAX_PACKETS; i++) {

    if(packets[i].state != NULL)
      if(!packets[i].timing || packets[i].timing && micros() - packets[i].registration > packets[i].timing) 
        packets[i].state = send_string(packets[i].device_id, packets[i].content); 
    
    if(packets[i].state == ACK) {
      if(packets[i].timing == 0)
        this->remove(i);
      else {
        packets[i].state = TO_BE_SENT;
        packets[i].registration = micros();
      }
    }
  }
}


/* Remove a packet from the send list: */

void ASK_Slang::remove(int packet_id) {
  packets[packet_id].content = NULL;
  packets[packet_id].state = NULL;
  packets[packet_id].device_id = NULL;
  packets[packet_id].registration = NULL;
}

/* Receiver side functions ------------------------------------------------------------------------------- */

/* Check if a byte is coming from the pin:
 If there is a 1 and is shorter then BIT_SPACER
 and after that comes a 0 probably a byte is coming */

int ASK_Slang::receive_byte() {
  float value = 0.5;
  unsigned long time = micros();

  while(micros() - time < BIT_SPACER && digitalRead(_input_pin))
    value = (value * 0.999)  + (digitalRead(_input_pin) * 0.001);
  
  time = micros();

  if(value > 0.5) {

    value = 0.5;
    while(micros() - time < BIT_WIDTH)
      value = (value * 0.999)  + (digitalRead(_input_pin) * 0.001);
    
    if(value < 0.5)
      return this->read_byte();
  }  

  return FAIL;
}

/* Read a byte from the pin */

uint8_t ASK_Slang::read_byte() {
  uint8_t byte_value = B00000000;
  
  for(uint8_t i = 0; i < 8; i++) {
    unsigned long time = micros();
    float value = 0.5;
    while(micros() - time < BIT_WIDTH)
      value = ((value * 0.999) + (digitalRead(_input_pin) * 0.001));
      
    byte_value += (value > 0.5) << i;
  }

  return byte_value;
}


/* Try to receive a string from the pin: */

int ASK_Slang::receive() {
  int package_length = MAX_PACKET_LENGTH;
  uint8_t CRC = 0;

  for (uint8_t i = 0; i < package_length; i++) {
    data[i] = this->receive_byte();

    if (data[i] == FAIL)
      return FAIL;

    if(i == 0 && data[i] != _device_id && data[i] != BROADCAST)
      return BUSY;

    if(i == 1)
      package_length = data[i];

    CRC ^= data[i];
  }

  if (!CRC) {
    if(_encryption) {
      for(int i = 0; i < package_length - 1; i++)
        if(i > 1) data[i - 2] = data[i];

      this->crypt((char*)data, true, 1);
    }
    return ACK;

  } else {
    return NAK;
  }
}

/* Try to receive a string from the pin repeatedly:
 receive() is executed in cycle with a for because is
 not possible to use micros() as condition (too long to be executed).
 micros() is then used in while as condition approximately every
 10 milliseconds (3706 value in for determines duration) */

int ASK_Slang::receive(unsigned long duration) {
  int response;
  unsigned long time = micros();
  while(micros() - time <= duration) {
    for(unsigned long i = 0; i < 3706; i++) {
      response = this->receive();
      if(response == ACK) { 
        if(data[2] == CMD)
          break;
        break;
      }
    }
  }
  return response;
}

