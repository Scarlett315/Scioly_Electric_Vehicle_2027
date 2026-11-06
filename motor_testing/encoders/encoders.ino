#include <Motoron.h>
MotoronI2C mc; // initialize motor controller

//pin numbers; motor #s 1/2 each with internal trigger A/B
#define ENC1A 5
#define ENC1B 6
#define ENC2A 1
#define ENC2B 2

// variables
int start_time = millis();
int pos_1 = 0;
int pos_2 = 0; 

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // ENCODER PIN INITIALIZATION
  pinMode(ENC1A, INPUT);
  pinMode(ENC1B, INPUT);
  pinMode(ENC2A, INPUT);
  pinMode(ENC2B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENC1A), read_encoder_1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2A), read_encoder_2, RISING);


  // MOTOR CONTROLLER (re)INITIALIZATION
  mc.reinitialize();   
  mc.disableCrc();  
  mc.clearResetFlag();  

  // Configure motor 1
  mc.setMaxAcceleration(1, 140);
  mc.setMaxDeceleration(1, 300);

  // Configure motor 2
  mc.setMaxAcceleration(2, 200);
  mc.setMaxDeceleration(2, 300);
}

void loop() {
  // run motors
  mc.setSpeed(1, 100);
  mc.setSpeed(2, 100);

  Serial.println(pos_1);
  //Serial.println(" ");
  //Serial.print(pos_2);

  // end condition; run for 60s
  if (millis() - start_time > 10000){
    while(true); // indef. loop
  }
}

void read_encoder_1(){
  int b = digitalRead(ENC1B);
  if(b>0){ // when the interrupt triggers, signal b is high  (clockwise)
    pos_1++;
  } else{ // when the interrupt triggers, signal b is low (counterclockwise)
    pos_1--;
  }
}

void read_encoder_2(){
  int b = digitalRead(ENC2B);
  if(b>0){ // when the interrupt triggers, signal b is high  (clockwise)
    pos_2++;
  } else{ // when the interrupt triggers, signal b is low (counterclockwise)
    pos_2--;
  }
}

void basic_read(){
  // run motors
  mc.setSpeed(1, 100);
  mc.setSpeed(2, 100);

  //read encoders & print
  int enc_1a = digitalRead(ENC1A);
  int enc_1b = digitalRead(ENC1B);
  int enc_2a = digitalRead(ENC2A);
  int enc_2b = digitalRead(ENC2B);

  Serial.print(enc_1a); // multiply for plot readability
  Serial.println();
  Serial.print(enc_1b); 
  Serial.println();

  Serial.print(enc_2a);
  Serial.println();
  Serial.print(enc_2b); 
  Serial.println();
 
  // end condition; run for 60s
  if (millis() - start_time > 10000){
    while(true); // indef. loop
  }
}
