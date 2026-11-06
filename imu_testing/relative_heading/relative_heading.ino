#include "Modulino.h"

// Create a ModulinoMovement
ModulinoMovement movement;

float x, y, z;
//float roll, pitch, yaw;
float omega; 
float theta = 0; // z-axis heading; the z-axis (left/right) is the important one here
float prev_time = 0; // time since last measurement
float dT = 0;
float dTheta = 0;


void setup() {
  Serial.begin(9600);
  Wire.begin();
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to movement sensor module
  movement.begin();
}

void loop() {
  if (millis() % 50 == 0){    // read data every 50 ms
    movement.update();        // Read new movement data from the sensor
    //z = movement.getZ();      
    omega = movement.getYaw() + 0.06; // angular velocity; this had an experimental error of ~0.06-0.12 for some reason

    Serial.print("angular velocity: ");
    Serial.println(omega);
    
    dT = millis() - prev_time; // calculate the change in time
    prev_time = millis();

    //Serial.print(" / change in time:  ");
    //Serial.println(dT);
    
    // integrate angular velocity with respect to time to get heading (approxmiation)
    dTheta = dT/1000 * omega;      // change in heading = change in time (seconds) x angular velocity
    theta += dTheta;               // calculate new heading
    
    //make the value wrap around (so that -180 < theta < 180)
    while (theta > 180){
      theta -= 360;
    }
    while (theta < -180){
      theta += 360;
    }

    Serial.println(theta);
  }
}

/*
// Linear acceleration (changes in velocity along the X, Y, and Z axes)
  x = movement.getX();
  y = movement.getY();
  z = movement.getZ(); //this is the important one -- left/right

  // Angular velocity (rate of rotation around the axes)
  roll = movement.getRoll();
  pitch = movement.getPitch();
  yaw = movement.getYaw(); // left/right

*/
