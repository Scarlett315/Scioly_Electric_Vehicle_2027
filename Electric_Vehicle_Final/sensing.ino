void IRAM_ATTR read_encoder_M1(){
  int b = digitalRead(ENC1B);
  if(b>0){ // when the interrupt triggers, signal b is high  (clockwise)
    pos_1++;
  } else{ // when the interrupt triggers, signal b is low (counterclockwise)
    pos_1--;
  }
}

void IRAM_ATTR read_encoder_M2(){
  int b = digitalRead(ENC2B);
  if(b>0){ // when the interrupt triggers, signal b is high  (clockwise)
    pos_2--;
  } else{ // when the interrupt triggers, signal b is low (counterclockwise)
    pos_2++;
  }
}

// reads new data in and updates heading -- do this as often as possible (prev. setting 50ms)
void update_heading(){
  movement.update();        // Read new movement data from the sensor
  omega = movement.getYaw() + DRIFT_CORRECTION; // angular velocity; this had an experimental error of ~0.06-0.12 for some reason
  //Serial.println(omega);

  //Serial.print("angular velocity: ");
  //Serial.println(omega);
  
  unsigned long now = millis();
  dT = now - prev_time; // calculate the change in time
  prev_time = now;

  // integrate angular velocity with respect to time to get heading (approxmiation)
  dTheta = dT/1000 * omega;      // change in heading = change in time (seconds) x angular velocity
  heading += dTheta  * ANGLE_CORRECTION;       // calculate new heading
  
  //make the value wrap around (so that -180 < theta < 180)
  heading = normalize_angle(heading);
}

// debug function for reading the heading!
bool read_heading(int ms){
  static int start_time;
  static bool initialized = false;
  static long last_print; 

  if (!initialized){
    start_time = millis();
    last_print = millis();
    initialized = true;

  }

  if (millis() - last_print > 500){
    Serial.print("Time: ");
    Serial.print(millis());
    Serial.print("| Heading: ");
    Serial.print(heading);
    Serial.print(" | Corrected Heading: ");
    Serial.println(corrected_heading);
    last_print = millis();
  }


  if (millis()- start_time > ms){
    return true;
  }

  return false;
}


