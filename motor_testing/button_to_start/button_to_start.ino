#include <Motoron.h>
MotoronI2C mc; // initialize motor controller

// constants / pin numbers
const int BUTTON_PIN = 21;  // the number of the pushbutton pin

// variables
bool started = false; // whether the button has been pushed or not to start the run
int buttonState = 0;  // variable for reading the pushbutton status

void setup() {
  Wire.begin();

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
  
  
  // PUSHBUTTON INITIALIZATION
  pinMode(BUTTON_PIN, INPUT); 
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTON_PIN);

  delay(100);
  Serial.print("button state:");
  Serial.println(buttonState);
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    Serial.print("Button pressed: running motors");
    delay(500); // wait 0.5s
    started = true;
  } 

  if (started){
    run_motors_test();
    started = false;
  }
  
}

void run_motors_test(){
  int start_time = millis();
  mc.setSpeed(1, 100);
  mc.setSpeed(2, 100);

  // stop giving power to motors after 10s
  delay(10000)
  mc.setSpeed(1, 0);
  mc.setSpeed(2, 0);
  return;
}
