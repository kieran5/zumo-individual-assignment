#include <ZumoMotors.h>
#include <Servo.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>

#define LED_PIN 13

// this might need to be tuned for different lighting conditions, surfaces, etc.
#define QTR_THRESHOLD  500 // microseconds

// Speed/duration settings
#define REVERSE_SPEED     200
#define TURN_SPEED        200
#define FORWARD_SPEED     200
#define REVERSE_DURATION  200
#define TURN_DURATION     300

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors(QTR_NO_EMITTER_PIN);
ZumoMotors motors;

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

bool detectLineInFrontOfZumo() {
  // TODO
  return false;  
}

void goForwardWithBorderDetectUntilCornerReached() {
    // While line in front of Zumo NOT detected keep running border detect code
    while(!detectLineInFrontOfZumo()) {
        // Go forward with border detect        
        reflectanceSensors.read(sensor_values);
        if (sensor_values[0] > QTR_THRESHOLD) {
          // if leftmost sensor detects line, reverse and turn to the right
          motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
          delay(REVERSE_DURATION);
          motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
          delay(TURN_DURATION);
          motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        }
        else if (sensor_values[5] > QTR_THRESHOLD) {
          // if rightmost sensor detects line, reverse and turn to the left
          motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
          delay(REVERSE_DURATION);
          motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
          delay(TURN_DURATION);
          motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        }
        else {
          // otherwise, go straight
          motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        }
      }

      // Once line in front of Zumo detected - will fall out of while loop and stop motors
      motors.setSpeeds(0, 0);

      // Send message that a corner has been reached to GUI

      
}

void playCountdown() {
  // play audible countdown
  for (int i = 0; i < 3; i++) {
    delay(1000);
    buzzer.playNote(NOTE_G(3), 200, 15);
  }
  
  delay(1000);
  buzzer.playNote(NOTE_G(4), 500, 15);  
  delay(1000); 
}

void setup() {
    // Initialise the reflectance sensors module
    reflectanceSensors.init();

    // Init LED pin to enable it to be turned on later
    pinMode(LED_PIN, OUTPUT);

    // Set up serial monitor for inital testing purposes
    Serial.begin(9600);
    Serial.println("waiting...");
}

void loop() {
  if(Serial.available()) {
    char valFromGUI = Serial.read();

    if(valFromGUI == 'W') {
      goForwardWithBorderDetectUntilCornerReached();
    }

    // Turn Zumo left 90 degrees (maybe change this to incrementally?)
    if(valFromGUI == 'L') {
      
    }

    // Turn Zumo right 90 degrees (maybe change this to incrementally?)
    if(valFromGUI == 'R') {
      
    }

    // Signal that Zumo has completed turn and can carry on forward
    if(valFromGUI == 'C') {
      goForwardWithBorderDetectUntilCornerReached();   
    }

    // Stop Zumo (for when you reach a room or side-corridor)
    if(valFromGUI == 'S') {
      motors.setSpeeds(0, 0);
    }

    // Signal that a room is about to be entered
    if(valFromGUI == 'Ro') {
      // Tell zumo whether room is on left or right of it
      if(valFromGUI == 'L') {
        
      }

      if(valFromGUI == 'R') {
        
      }
    }

    // Signal that a side-corridor is about to be entered
    if(valFromGUI == 'Co') {
      
    }
  }  
}
