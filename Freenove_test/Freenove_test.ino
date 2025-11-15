/**********************************************************************
  Product     : Freenove 4WD Car for Raspberry Pi Pico (W)
  Auther      : www.freenove.com
  Modification: 2023/07/10
**********************************************************************/
#include "Freenove_4WD_Car_For_Pico_W.h"
#include <math.h>

bool obstacleLock = false;       // true when object ≤ SAFE_DISTANCE
unsigned long lastCheck = 0;
const int SAFE_DISTANCE = 30;    // cm
const int CHECK_INTERVAL = 100;  // ms between sonar checks
int servAng = 90;  // Initial servo angle

struct Position {
  float x;
  float y;
};

Position previousPosition = {0, 0};
unsigned long previousTime = 0;

float previousDistance = 0;  // Store the previous distance measurement
int velocityPrintCounter = 5;  // Counter to control velocity printing
unsigned long previousTime = 0; // Store the previous time measurement

float calculateVelocity(float currentDistance) {
  // Compute dt in seconds
  unsigned long currentTime = millis();
  float dt = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;
  if (dt <= 0) return 0;

  // Convert servo angle offset from forward direction to radians
  float theta = absa(servAng - 90) * M_PI / 180.0;

  // Project distances along forward direction
  float posNow = currentDistance * cos(theta);
  float posPrev = previousDistance * cos(theta);

  // Change in distance along heading
  float dx = posNow - posPrev;

  // Update
  previousDistance = currentDistance;

  // Velocity in cm per second
  float velocity = -dx / dt;

  // Print every 5 calls
  velocityPrintCounter--;
  if (velocityPrintCounter == 0) {
    Serial.print("Velocity: ");
    Serial.println(velocity);
    velocityPrintCounter = 5;
  }

  return velocity;
}
void setup() {
  Serial.begin(9600);
  Servo_Setup();
  Motor_Setup();
  Ultrasonic_Setup();
  Servo_1_Angle(90);
  delay(500);
}

/*void loop()
{
  // Servo 1 motion path; 90°- 30°- 150°- 90°
   Servo_Sweep(1, 90, 30);
   Servo_Sweep(1, 30, 150);
   Servo_Sweep(1, 150, 90);
}*/





void loop() {
  unsigned long now = millis();

  // Ultrasonic check every 100 ms
  if (now - lastCheck >= CHECK_INTERVAL) {
    lastCheck = now;
    float distance = Get_Sonar();
    //Serial.print("Distance: ");
    //Serial.println(distance);

    // Calculate velocity based on distance change
    calculateVelocity(distance);

    // If obstacle detected, stop immediately
    if (distance <= SAFE_DISTANCE && !obstacleLock) {
      obstacleLock = true;
      //Serial.println("Obstacle detected, stopping immediately!");
      Motor_Move(0, 0);
    }

    // If obstacle cleared, unlock forward
    if (obstacleLock && distance > SAFE_DISTANCE) {
      obstacleLock = false;
      //Serial.println("Path clear, forward allowed again.");
    }
  }

  // Handle serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.equalsIgnoreCase("forward")) {
      // Block forward if obstacle is present
      if (obstacleLock) {
        //Serial.println("Forward blocked, obstacle ahead.");
        return;
      }
      //Serial.println("Moving forward...");
      Motor_Move(-40, -40);
    } else if (command.equalsIgnoreCase("backward")) {
      //Serial.println("Moving backward...");
      Motor_Move(40, 40);
    } else if (command.equalsIgnoreCase("left")) {
      //Serial.println("Turning left...");
      Motor_Move(60, -60);
    } else if (command.equalsIgnoreCase("right")) {
      //Serial.println("Turning right...");
      Motor_Move(-60, 60);
    } else if (command.equalsIgnoreCase("stop")) {
      //Serial.println("Stopping...");
      Motor_Move(0, 0);
    } else if (command.equalsIgnoreCase("leftS")) {
      //Serial.println("Stopping...");
      Servo_1_Angle(servAng - 10);
      servAng -= 10;
    } else if (command.equalsIgnoreCase("rightS")) {
      //Serial.println("Stopping...");
      Servo_1_Angle(servAng + 10);
      servAng += 10;
    } else if (command.equalsIgnoreCase("servo_reset")) {
      Servo_1_Angle(90);
      servAng = 90;
    } else {
      //Serial.println("Unknown command. Try: forward / backward / left / right / stop");
      Motor_Move(0, 0);
    }
  }
}
