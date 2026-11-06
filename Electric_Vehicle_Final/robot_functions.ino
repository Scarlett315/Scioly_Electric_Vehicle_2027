// CONVERSIONS
// output shaft rotation = input shaft rotations / gear ratio

// encoder counts --> wheel angle --> distance traveled
float counts_to_angle(int encoder_counts){ return ((float) encoder_counts / (float) (ENCODER_CPR)) / (float) GEAR_RATIO * 360.0; }
float angle_to_dist(float wheel_angle){ return (wheel_angle / 360.0) * (2 * PI * WHEEL_RADIUS) ;}

// target distance --> target wheel angle --> target encoder counts
float angle_to_counts(float angle){ return angle / 360.0 * (float) GEAR_RATIO * (float) (ENCODER_CPR); }
float dist_to_angle(float target_dist){ return target_dist / (2 * PI * WHEEL_RADIUS) * 360; }


float normalize_angle(float angle) {
  while (angle >= 180.0) angle -= 360.0;
  while (angle <  -180.0) angle += 360.0;
  return angle;
}

float shortest_angle(float target, float current) {
  return normalize_angle(target - current);
}

// ------------------- PD --------------------------
float Kp_heading = 6.0;  
float Kd_heading = 2.2;

// --------------------------- SIMPLE DRIVE -----------------------------------
// spin motor for a certain time
void rotate_time_simple(int motor, int speed, float target_time){
  float target_time_ms = target_time * 1000;
  int start_time = millis();
  mc.setSpeed(motor, speed);

  if (target_time_ms >= start_time){
    mc.setSpeed(motor, 0);
  }
  return;
}

// spin a single motor to get to a target distance (m) -- besides initial conversion everything is in encoder counts
void rotate_dist(int motor, float speed, float target_dist, bool debug){
  float target_counts = angle_to_counts(dist_to_angle(target_dist)); // how many encoder counts to get to that position
  if (debug){
    Serial.print("Target Counts: " );
    Serial.print(target_counts);
  }
  
  mc.setSpeed(motor, speed); // set motor speed!
  float pos_initial; 

  if (motor == 1){
    pos_initial = pos_1;

    // keep running motor (yield) while current position < initial + target change in position
    while (abs(pos_1) < abs(target_counts + pos_initial)){

      static unsigned long lastPrint = 0;
      if (debug && millis() - lastPrint > 100) {
        Serial.print("Encoder Position: ");
        Serial.println(pos_1);
        lastPrint = millis();
      }
      
      yield();
    } 

  } 
  
  if (motor == 2){
    pos_initial = pos_2;

    while (abs(pos_2) < abs(target_counts + pos_initial)){

      static unsigned long lastPrint = 0;
      if (debug && millis() - lastPrint > 100) {
        Serial.print("Encoder Position: ");
        Serial.println(pos_2);
        lastPrint = millis();
      
      yield();
      }
    }
  }

  mc.setSpeed(motor, 0); //stop motor

}

// drive -- spin both motors in the same direction
bool drive_simple(float speed, float target_dist, bool debug){
  static bool initialized = false;
  static long target_counts; 
  static long pos_initial_1, pos_initial_2;
  static unsigned long last_print;


  // initialization
  if (!initialized){
    target_counts = angle_to_counts(dist_to_angle(target_dist)); // how many encoder counts to get to that position
    pos_initial_1 = pos_1;
    pos_initial_2 = pos_2;
    last_print = 0;

    if (debug){
      Serial.print("Target counts: ");
      Serial.print(target_counts);
      Serial.print(" | pos_initial_1: ");
      Serial.print(pos_initial_1);
      Serial.print(" | pos_initial_2: ");
      Serial.println(pos_initial_2);
    }

    mc.setSpeed(1, speed);
    mc.setSpeed(2, -speed);

    initialized = true; 
  }

  // error calculation + stop condition
  long delta_left  = pos_1 - pos_initial_1; 
  long delta_right = pos_2 - pos_initial_2;
  long avg_delta = (delta_left + delta_right) / 2;
  long error = target_counts - avg_delta;

  if(abs(error) < 10){
    mc.setSpeed(1, 0);
    mc.setSpeed(2, 0);
    return true;
  }

  // print encoder positions every 1s
  if (debug && millis() - last_print > 1000){
    Serial.print("Time: ");
    Serial.print(millis());
    Serial.print("ms | encoder pos_1: ");
    Serial.print(pos_1);
    Serial.print(" | encoder pos_2: ");
    Serial.println(pos_2);

    last_print = millis();
  }

  return false;
}

// tank turn a certain number of degrees
bool tank_turn_simple(float speed, float target_angle, bool debug){
  static bool initialized = false;
  static float initial_angle;
  static unsigned long last_print = 0;
  static float turn_speed;

  // initialization/get motors running + decide which way would be less distance to turn
  if (!initialized){
    initial_angle = heading;

    if (debug){
      Serial.print("Initial Heading: ");
      Serial.println(heading);
      last_print = 0;
    }

    turn_speed = constrain(abs(speed), 0, 400);
    initialized = true; 
  }

  // calculate error 
  float error = shortest_angle(target_angle, heading);

  // which direction motors should turn
  if (error > 0) {
    mc.setSpeed(1,  turn_speed);
    mc.setSpeed(2, -turn_speed);
  } else {
    mc.setSpeed(1, -turn_speed);
    mc.setSpeed(2,  turn_speed);
  }

  // stop condition
  if (abs(error) <= 2.0) {
    mc.setSpeed(1, 0);
    mc.setSpeed(2, 0);
    initialized = false; //allow for new turns
    return true;
  }

  // debugging! 
  if (debug){
    if (millis() - last_print > 1000) {
      Serial.print("Time: ");
      Serial.print(millis());
      Serial.print("| Current Angle: ");
      Serial.println(heading);
      last_print = millis();
    }
  } 
  
  return false;
}

// --------------------------- DRIVE WITH HEADING CORRECTION -----------------------------------

bool drive_towards(float speed, float target_dist, float target_angle, bool debug){
  static bool initialized = false;
  static long target_counts; 
  static long pos_initial_1, pos_initial_2;
  //static float target_angle;
  static float prev_heading_error;
  static unsigned long last_print;
  static unsigned long last_time_correction;

  //initialization
  if (!initialized){
    target_counts = angle_to_counts(dist_to_angle(target_dist)); // how many encoder counts to get to that position
    pos_initial_1 = pos_1;
    pos_initial_2 = pos_2;

    //target_angle = target_angle_init;
    prev_heading_error = 0;
    last_time_correction = millis();
    last_print = millis();

    if (debug){
      Serial.print("Target counts: ");
      Serial.print(target_counts);
      Serial.print(" | pos_initial_1: ");
      Serial.print(pos_initial_1);
      Serial.print(" | pos_initial_2: ");
      Serial.println(pos_initial_2);
    }

    //mc.setSpeed(1, speed);
    //mc.setSpeed(2, speed);

    initialized = true; 

  }

  // distance error calculation
  long delta_left  = pos_1 - pos_initial_1; 
  long delta_right = pos_2 - pos_initial_2;
  long avg_delta = (delta_left + delta_right) / 2;
  long dist_error = target_counts - avg_delta;

  // Stop condition
  if(abs(dist_error) < 10){
    mc.setSpeed(1, 0);
    mc.setSpeed(2, 0);

    //reset
    initialized = false;
    //current_speed = 0;
    prev_heading_error = 0;
    return true;
  }

  
  // heading error calculation & implementation of PD
  unsigned long now = millis();
  float dt = (now - last_time_correction) / 1000.0; // change in time
  if (dt <= 0) dt = 1e-3; // prevent mistakes/huge corrections
  last_time_correction = now;

  float heading_error = shortest_angle(target_angle, heading); //proportional
  float d_error = (heading_error - prev_heading_error) / dt; //derivative (dw/dt)
  prev_heading_error = heading_error;

  float correction = Kp_heading * heading_error + Kd_heading * d_error; // PD correction calculation

  
  float left_cmd  = constrain(speed - correction,  80, 400);
  float right_cmd = constrain(speed + correction, 80, 400);
  mc.setSpeed(1, left_cmd);
  mc.setSpeed(2, -right_cmd);

  // debug variables!
  if (debug && millis() - last_print > 1000){
    Serial.print("Time: ");
    Serial.print(millis());
    Serial.print("ms | ENCODERS - pos_1: ");
    Serial.print(pos_1);
    Serial.print(" & pos_2: ");
    Serial.print(pos_2);

    Serial.print(" | PID - heading: ");
    Serial.print(heading);
    Serial.print(" & heading error: ");
    Serial.print(heading_error);
    Serial.print(" & correction: ");
    Serial.print(correction);
    Serial.print(" & left_cmd: ");
    Serial.print(left_cmd);
    Serial.print(" & right_cmd: ");
    Serial.println(right_cmd);
  
    last_print = millis();
  }

  return false;

}





