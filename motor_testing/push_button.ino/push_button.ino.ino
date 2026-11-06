// constants won't change. They're used here to set pin numbers:
const int buttonPin = 47;  // the number of the pushbutton pin

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  delay(200);
  Serial.print("button state:");
  Serial.println(buttonState);
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    Serial.print("Button pressed \n");

  } 
  //else {
    // turn LED off:
    //digitalWrite(ledPin, LOW);
  //}
}
