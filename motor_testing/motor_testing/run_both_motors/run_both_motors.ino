// This example shows a simple way to control the
// Motoron Motor Controller using I2C.
//
// The motors will stop but automatically recover if:
// - Motor power (VIN) is interrupted, or
// - A temporary motor fault occurs, or
// - A command timeout occurs.
//
// The motors will stop until you power cycle or reset your
// Arduino if:
// - The Motoron experiences a reset.
//
// If a latched motor fault occurs, the motors
// experiencing the fault will stop until you power cycle motor
// power (VIN) or cause the motors to coast.

#include <Motoron.h>

MotoronI2C mc;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // Reset the controller to its default settings, then disable
  // CRC.  The bytes for each of these commands are shown here
  // in case you want to implement them on your own without
  // using the library.
  mc.reinitialize();    // Bytes: 0x96 0x74
  mc.disableCrc();      // Bytes: 0x8B 0x04 0x7B 0x43

  // Clear the reset flag, which is set after the controller
  // reinitializes and counts as an error.
  mc.clearResetFlag();  // Bytes: 0xA9 0x00 0x04

  // By default, the Motoron is configured to stop the motors if
  // it does not get a motor control command for 1500 ms.  You
  // can uncomment a line below to adjust this time or disable
  // the timeout feature.
  // mc.setCommandTimeoutMilliseconds(1000);
  // mc.disableCommandTimeout();

  // Configure motor 1
  mc.setMaxAcceleration(1, 140);
  mc.setMaxDeceleration(1, 300);

  // Configure motor 2
  mc.setMaxAcceleration(2, 200);
  mc.setMaxDeceleration(2, 300);

}

void loop()
{
  
  mc.setSpeed(1, 100);

  mc.setSpeed(2, 100);

  printMotoronStatusFlags();
  delay(1000);
}

void printMotoronStatusFlags()
{
  Serial.println(F("=== Motoron Status Flags ==="));



  Serial.print(F("Motor fault (latched): "));
  Serial.println(mc.getMotorFaultLatchedFlag());

  Serial.print(F("No power (latched): "));
  Serial.println(mc.getNoPowerLatchedFlag());

  Serial.print(F("Motor faulting: "));
  Serial.println(mc.getMotorFaultingFlag());

  Serial.print(F("No power: "));
  Serial.println(mc.getNoPowerFlag());

  Serial.print(F("Error active: "));
  Serial.println(mc.getErrorActiveFlag());

  Serial.print(F("Motor output enabled: "));
  Serial.println(mc.getMotorOutputEnabledFlag());

  Serial.print(F("Motor driving: "));
  Serial.println(mc.getMotorDrivingFlag());

  Serial.println(F("============================"));
}
