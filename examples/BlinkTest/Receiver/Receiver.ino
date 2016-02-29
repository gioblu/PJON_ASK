#include <PJON_ASK.h>

// network(Arduino pin used, selected device id)
PJON_ASK network(11, 12, 44);

void setup() {
  Serial.begin(9600);
  network.set_receiver(receiver_function);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
};

static void receiver_function(uint8_t length, uint8_t *payload) {
  if(payload[0] == 'B') {
    Serial.println("BLINK");
    digitalWrite(13, HIGH);
    delay(30);
    digitalWrite(13, LOW);
  }
}

void loop() {
  network.receive(1000);
};
