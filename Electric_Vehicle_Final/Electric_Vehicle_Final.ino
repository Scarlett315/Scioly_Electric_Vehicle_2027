#include <Motoron.h>
#include "Modulino.h"

MotoronI2C mc; // initialize motor controller
ModulinoMovement movement;


// ----------- PIN NUMBERS  ------------------
int BUTTON_PIN = 21;  // the number of the pushbutton pin
//encoders; motor #s 1/2 each with internal trigger A/B
#define ENC1A 5
#define ENC1B 6
#define ENC2A 13
#define ENC2B 14

// ----------- CONSTANTS & VARIABLES --------------------
bool run_complete = false;

// hardware constants -- everything length-related in meters
const float WHEEL_RADIUS = 0.0254;
const int ENCODER_CPR = 3;
const int GEAR_RATIO = 51;
const float AXIAL_DIST = 0.1; //distance beteen the wheels (m)

const float ANGLE_CORRECTION = 1.16;
const float DRIFT_CORRECTION = 0.55;

// position/encoders -- in encoder counts
int start_time;
int last_heading_update;
volatile long pos_1 = 0;
volatile long pos_2 = 0; 

// angle/heading 
float x, y, z;
float omega; 


volatile float heading = 0; //z-axis heading; -180 < heading < 180
float corrected_heading = 0;

float prev_time = 0; // time since last measurement
float dT = 0;
float dTheta = 0;

// start button
bool started = false; 
int button_state = 0; 

//run constants
// angle is 30 degrees
const float TRACK_DIST = 9.5; // 7m < d < 20m
const float TARGET_TIME = 20; // 10s < t < 20s
const float TARGET_CAN_DIST = 0.8; // distance from center line to middle of cans (m)
const float INITIAL_RUN_DIST = 0.5;  

const float FINAL_OFFSET = 0.05; // final distance is usually off by a few cm
const float TURN_SPEED = 300; 

//run distances
float dist_angled = 2 * TARGET_CAN_DIST;
float dist_straight = TRACK_DIST - INITIAL_RUN_DIST - 1.73205 * dist_angled;

// ---------- DECLARATIONS --------------
void rotate_dist(int motor, float speed, float target_dist, bool debug=false);
bool drive_simple(float speed, float target_dist, bool debug=false);
bool tank_turn_simple(float speed, float target_angle, bool debug=false);
bool drive_towards(float speed, float target_dist, float target_angle=heading, bool debug=false);
bool read_heading(int ms);

void IRAM_ATTR read_encoder_M1();
void IRAM_ATTR read_encoder_M2();
void update_heading();

// -------------------------------------------------------------------------------------

// motion states / "steps"
enum MotionState {
  WAITING,
  DRIVE_INITIAL,
  DRIVE_OUT,
  DRIVE_FORWARD,
  DRIVE_BACK,
  DONE,
  DRIVE_TEST
};

MotionState motion_state = WAITING;


// Setup & Run!
void setup() {
  setup_hardware();
  delay(100);
  Serial.println("Everything has been set up!");
}

void loop() {
  //update frequency = 1 / 50ms = 20Hz
  if (millis() - last_heading_update >= 50){
    update_heading();
    //corrected_heading = heading * ANGLE_CORRECTION;
    last_heading_update = millis();
  }
  
  // ---- motion state  ----
  switch (motion_state) {
    case WAITING:
      if (await_button()){
        motion_state = DRIVE_INITIAL;
      }
      break;

    case DRIVE_INITIAL:
    if (drive_towards(300, INITIAL_RUN_DIST, 0, true)) {  
      motion_state = DRIVE_OUT;
    }
    break;

    case DRIVE_OUT:
      if (drive_towards(TURN_SPEED, dist_angled, 30, true)) {
        motion_state = DRIVE_FORWARD;
      }
      break;

    case DRIVE_FORWARD:
      if (drive_towards(300, dist_straight, 0, true)) {
        motion_state = DRIVE_BACK;
      }
      break;

    case DRIVE_BACK:
    if (drive_towards(TURN_SPEED, dist_angled + FINAL_OFFSET, -30, true)) {
      motion_state = DONE;
    }
    break;

    case DONE:
      break;
  }
}

// -------------------------------------------------------------------------------------

void setup_hardware(){
  Serial.begin(9600);
  //while (!Serial) { delay(10); }
  last_heading_update = millis();

  delay(2000);
  Wire.begin();
  delay(2000); 
  
  // MOTOR CONTROLLER (re)INITIALIZATION
  mc.reinitialize();   
  mc.disableCrc();  
  mc.clearResetFlag();  
  mc.disableCommandTimeout();  // disable timeout

  // Configure motor 1
  mc.setMaxAcceleration(1, 200);
  mc.setMaxDeceleration(1, 200);

  // Configure motor 2
  mc.setMaxAcceleration(2, 200);
  mc.setMaxDeceleration(2, 200);

  // ENCODER PIN INITIALIZATION
  pinMode(ENC1A, INPUT);
  pinMode(ENC1B, INPUT);
  pinMode(ENC2A, INPUT);
  pinMode(ENC2B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENC1A), read_encoder_M1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2A), read_encoder_M2, RISING);

  // IMU INITIALIZATION
  Modulino.begin();
  movement.begin();
  
  // PUSHBUTTON INITIALIZATION
  pinMode(BUTTON_PIN, INPUT); 
}

// wait until button is pressed to start the run
bool await_button(){
  delay(100);

  button_state = digitalRead(BUTTON_PIN);

  // check if the pushbutton is pressed
  if (button_state == HIGH) {
    Serial.println("Run started!!!");
    delay(500); // wait 0.5s

    //reset variables
    start_time = millis();
    pos_1 = 0;
    pos_2 = 0;
    heading = 0;
    return true;
  } 
  return false;

}

