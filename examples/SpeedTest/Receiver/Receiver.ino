#include <PJON_ASK.h>

float test;
float mistakes;

// network(Arduino pin used, selected device id)
PJON_ASK network(11, 12, 44);

void setup() {
  Serial.begin(115200);
  network.set_receiver(receiver_function);
};

static void receiver_function(uint8_t length, uint8_t *payload) {
 // Primitive connection quality graph 
  Serial.print("-");
}

void loop() {
  long time = millis();
  int response = 0; 
  while(millis() - time < 1000) {
    response = network.receive(1000);
    if(response == ACK)
      test++;
    if(response == NAK)
      mistakes++;
  }
  
  Serial.println();
  Serial.print("Absolute com speed: ");
  Serial.print(test * 13);
  Serial.print(" B/s |Practical bandwidth: ");
  Serial.print(test * 10);
  Serial.print(" B/s |Packets sent: ");
  Serial.print(test);
  Serial.print(" |Mistakes ");
  Serial.print(mistakes);
  Serial.print(" |Accuracy: ");
  Serial.print(100 - (100 / (test / mistakes)));
  Serial.print(" %");
  Serial.println();
  
  if(mistakes > test / 4 || test == 0) {
    Serial.println("Check wiring!");
    Serial.println("1) Check wiring connections of both modules.");
    Serial.println("2) Transmitter voltage source has to be regulated.");
  }

  test = 0; 
  mistakes = 0;
};