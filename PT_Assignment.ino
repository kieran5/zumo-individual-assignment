#include <ZumoMotors.h>
#include <Servo.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>

#define LED_PIN 13

// this might need to be tuned for different lighting conditions, surfaces, etc.
#define QTR_THRESHOLD  250 // microseconds

// Speed/duration settings
#define REVERSE_SPEED     200
#define TURN_SPEED        200
#define FORWARD_SPEED     200
#define REVERSE_DURATION  100
#define TURN_DURATION     200
#define FORWARD_DURATION  100

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors(QTR_NO_EMITTER_PIN);
ZumoMotors motors;

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

boolean firstTimeWPressed = false;

void setup() {
    // Initialise the reflectance sensors module
    reflectanceSensors.init();

    // Init LED pin to enable it to be turned on later
    pinMode(LED_PIN, OUTPUT);

    // Set up serial port to enable Arduino to communicate with GUI
    Serial.begin(9600);

    // Call method to create initial connection to GUI
    establishConnectionToGUI();
}

void loop() {
  // Check if there is any input detected from GUI
  if(Serial.available() > 0) {
    String valFromGUI = Serial.readString();
    

    // Make Zumo go forward with border detection
    if(valFromGUI == "W") {
      Serial.println("W received by Zumo!");

      // Checks if first time 'W' pressed
      if(!firstTimeWPressed) {
        playCountdown();
        goForwardWithBorderDetectUntilCornerReached();
        firstTimeWPressed = true;         
      }
      else {
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        delay(FORWARD_DURATION);
        motors.setSpeeds(0, 0);           
      }
    }

    // Turn Zumo left 90 degrees (maybe change this to incrementally?)
    if(valFromGUI == "A") {
      Serial.println("A received by Zumo!");
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);                
    }

    // Stop Zumo (for when you reach a room or side-corridor)
    if(valFromGUI == "S") {
      Serial.println("S received by Zumo!");
      motors.setSpeeds(0, 0);
      
    }

    // Turn Zumo right 90 degrees (maybe change this to incrementally?)
    if(valFromGUI == "D") {
      Serial.println("D received by Zumo!");    
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(TURN_DURATION);
      motors.setSpeeds(0, 0);  
    }

    // Reverse control for Zumo - in case needed when turning corners under human control
    if(valFromGUI == "Rev") {
      Serial.println("Rev received by Zumo!");
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);
    }

    // Signal that Zumo has completed turn and can carry on forward
    if(valFromGUI == "C") {
      Serial.println("C received by Zumo!");
      goForwardWithBorderDetectUntilCornerReached();        
      
      motors.setSpeeds(0, 0);  
    }

    // Signal that a room is about to be entered
    if(valFromGUI == "Ro") {
      Serial.println("Ro received by Zumo!");
      // Tell zumo whether room is on left or right of it
      if(valFromGUI == "L") {
        
      }

      if(valFromGUI == "R") {
        
      }
    }

    // Signal that a side-corridor is about to be entered
    if(valFromGUI == "Co") {
      Serial.println("Co received by Zumo!");
      
    }

    // Signal that the end of the track has been reached
    if(valFromGUI == "E") {
      Serial.println("E received by Zumo!");
    }
  }  
}

void establishConnectionToGUI() {
  // Function to create handshake between Processing GUI and Arduino
  // Allows data to flow both ways
  while(Serial.available() <= 0) {
    // Send a 'Z' to create connection
    // This will be an unused character in this system
    Serial.println("Z");
    delay(300);
  }
}

void goForwardWithBorderDetectUntilCornerReached() {
    // Intial reading of reflectance sensors
    reflectanceSensors.read(sensor_values);

    // Initialise boolean to check when user manually presses stop
    boolean stopPressed = false;
    
    // While line in front of Zumo NOT detected keep running border detect code
    while(!(sensor_values[0] > QTR_THRESHOLD && sensor_values[5] > QTR_THRESHOLD)) {
        // Go forward with border detect

        // Reading serial point in case user needs to manually stop Zumo & break out of while loop
        if(Serial.read() == 'S') {
          stopPressed = true;
          break;
        }
        
        
        Serial.println("Sensor value 0: " + String(sensor_values[0]));
        Serial.println("Sensor value 5: " + String(sensor_values[5]));

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

        // Reinitialise reflectance sensors
        reflectanceSensors.read(sensor_values);
      }

      // Once line in front of Zumo detected - will fall out of while loop and stop motors
      motors.setSpeeds(0, 0);

      // If else statement to check what message to display to user on GUI
      if(stopPressed) {
        Serial.println("Zumo stopped by user.");        
      }
      else {
        // Send message that a corner has been reached to GUI     
        Serial.println("Zumo has reached a corner.");        
      }
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
