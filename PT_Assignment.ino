#include <ZumoMotors.h>
#include <Servo.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>

// Includes for standard C++ STL required for use of vectors (https://github.com/maniacbug/StandardCplusplus)
#include <StandardCplusplus.h>
#include <serstream>
#include <string>
#include <vector>
#include <iterator>

using namespace std;

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

#define TURN_180_SPEED         370
#define TURN_180_DURATION      370

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors(QTR_NO_EMITTER_PIN);
ZumoMotors motors;

// Initialise sensor variables to store values from reflectance sensors
#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

// Assign trig and echo pin numbers for ultra sonic sensor
const int trigPin = 2;
const int echoPin = 6;

// Initialise boolean to check if W button press is first in the current run
// Allows user to get Zumo off of start line without need of an additional button
boolean firstTimeWPressed = false;

// Initialise corridor and room count variables
// corridorID starts at 1 as the Zumo will start in a corridor (ID = 1)
// Zumo will not be in a room until it comes across one though, hence starting on 0
// These variables are needed so that the class instances are able to increment their ID's
int corridorCount = 1;
int roomCount = 0;

// Initialise current corridor variable - allows Zumo to always knows what corridor it is in
int currentCorridor = 0;

// Check whether 180 turn has been made at end of sub corridor or end of track
bool turnComplete = false;

class Corridor {
  private:
    int id;
    char corridorSide;
    bool subCorridorFlag;
    int previousCorridorID;

  public:
    Corridor();
    int getID();
    void setSide(char);
    char getSide();
    void setSubCorridorFlag();
    bool getSubCorridorFlag();
    void setPreviousCorridorID(int);
    int getPreviousCorridorID();
};

Corridor::Corridor() {
  id = corridorCount;
  corridorSide = 'A';
  subCorridorFlag = false;
}

int Corridor::getID() {
  return this->id;
}

void Corridor::setSide(char side) {
  corridorSide = side; 
}

char Corridor::getSide() {
  return this->corridorSide;
}

void Corridor::setSubCorridorFlag() {
  subCorridorFlag = true;  
}

bool Corridor::getSubCorridorFlag() {
  return this->subCorridorFlag;
}

void Corridor::setPreviousCorridorID(int id) {
  previousCorridorID = id;
}

int Corridor::getPreviousCorridorID() {
  return this->previousCorridorID;
}

class Room {
  private:
    int id;
    char roomSide;
    int corridorID;
    bool objectFound;

  public:
    Room();
    int getID();
    void setSide(char);
    char getSide();
    void setCorridor(int);
    int getCorridor();
    void setObjectFound(bool);
    bool getObjectFound();
};

Room::Room() {
  id = roomCount;
  roomSide = 'A';
  objectFound = false;
}

int Room::getID() {
  return this->id;
}

void Room::setSide(char side) {
  roomSide = side; 
}

char Room::getSide() {
  return this->roomSide;
}

void Room::setCorridor(int id) {
  corridorID = id;
}

int Room::getCorridor() {
  return this->corridorID;
}

void Room::setObjectFound(bool found) {
  objectFound = found;
}

bool Room::getObjectFound() {
  return this->objectFound;
}

// Intialise vectors for storing each instance of a room or corridor that is created
// We can quickly access these vectors to get data for any particular room or corridor
// Use of vectors allows ease of use for task 5 as we can read through the vectors from 
// last element backwards until we get back to the start of the track
std::vector<Room> rooms;
std::vector<Corridor> corridors;


void setup() {
    // Initialise the reflectance sensors module
    reflectanceSensors.init();

    // Init LED pin to enable it to be turned on later
    pinMode(LED_PIN, OUTPUT);

    // Set up serial port to enable Arduino to communicate with GUI
    Serial.begin(9600);

    // Call method to create initial connection to GUI
    establishConnectionToGUI();

    // Zumo will start in a corridor, will create this first corridor instance and store in vector
    corridors.push_back(Corridor::Corridor());
    currentCorridor = corridors.back().getID();
}

void loop() {  
  // Check if there is any input detected from GUI
  if(Serial.available() > 0) {
    char valFromGUI = Serial.read();
    

    // Make Zumo go forward with border detection
    if(valFromGUI == 'W') {
      Serial.println("W received by Zumo!");
      
      // Checks if first time 'W' pressed
      if(!firstTimeWPressed) {
        playCountdown();
        goForwardWithBorderDetectUntilCornerReached();
        firstTimeWPressed = true;         
      }
      // If not first time 'W' pressed, this control will just be used for manual forward control
      else {
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        delay(FORWARD_DURATION);
        motors.setSpeeds(0, 0);           
      }
    }

    // Turn Zumo to left slowly until stopped by user
    if(valFromGUI == 'A') {
      Serial.println("A received by Zumo!");

      // Set start time so Zumo is able to calculate duration of turn
      unsigned long startTime = millis();

      // Reading serial point waiting for user to press stop when the Zumo has turned enough
      while(Serial.read() != 'S') {
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);          
      }

      // Stop the Zumo, and calculate duration, ready to store in an object for task 5
      motors.setSpeeds(0, 0);
      unsigned long turnDuration = millis() - startTime;      
      Serial.println("Turn Duration: " + String(turnDuration));
    }

    // Stop Zumo (for when you reach a room or side-corridor)
    if(valFromGUI == 'S') {
      Serial.println("S received by Zumo!");
      motors.setSpeeds(0, 0);
      
    }

    // Turn Zumo to right (good idea to change this to 90 degree turn?)
    if(valFromGUI == 'D') {
      Serial.println("D received by Zumo!");

      // Set start time so Zumo is able to calculate duration of turn
      unsigned long startTime = millis();

      // Reading serial point waiting for user to press stop when the Zumo has turned enough
      while(Serial.read() != 'S') {
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);          
      }

      // Stop the Zumo, and calculate duration, ready to store in an object for task 5
      motors.setSpeeds(0, 0);
      unsigned long turnDuration = millis() - startTime;      
      Serial.println("Turn Duration: " + String(turnDuration));
    }

    // Reverse control for Zumo - in case needed when turning corners under human control
    if(valFromGUI == 'X') {
      Serial.println("Rev received by Zumo!");
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);

    }

    // Signal that Zumo has completed turn and can carry on forward
    if(valFromGUI == 'C') {
      Serial.println("C received by Zumo!");
      goForwardWithBorderDetectUntilCornerReached();
    }

    // Signal that a room is about to be entered
    if(valFromGUI == 'N') {
      Serial.println("Ro received by Zumo!");

      // Increment room count variable to assign new room with own unique ID
      ++roomCount;
      rooms.push_back(Room::Room());
      
      // Tell zumo whether room is on left or right of it
      Serial.println("Is the room on the left or right?");
      char side = Serial.read();
      // Wait until user has picked left or right before setting on object
      // Then continuing to display message to the user of what they have selected
      while(side != 'L' || side != 'R') { 
        side = Serial.read();
        
        if(side == 'L') {
          rooms.back().setSide('L');
          break;                        
        }
        if(side == 'R') {
          rooms.back().setSide('R');  
          break;      
        }    
      }

      // Set the current corridor of the room entered so Zumo remembers where it is on the course
      rooms.back().setCorridor(currentCorridor);
            
      Serial.println("New room about to be entered on " + String(rooms.back().getSide()) + " in corridor " + String(rooms.back().getCorridor()) + " - Room ID: " + String(rooms.back().getID()));
    }

    // Signal that a side-corridor is about to be entered
    if(valFromGUI == 'M') {
      Serial.println("Co received by Zumo!");

      // Take note of previous corridor ID so new corridor can record this information for use when new corridor is exited again
      int prevCorridorID = currentCorridor;

      // Increment corridor count variable to assign new corridor with own unique ID
      ++corridorCount;
      corridors.push_back(Corridor::Corridor());

      // Update current corridor variable so Zumo knows where it is
      currentCorridor = corridors.back().getID();

      // Set previous corridor ID so Zumo can keep this in memory for when it exits back on to previous corridor
      corridors.back().setPreviousCorridorID(prevCorridorID);
      
      // Tell zumo whether corridor is on left or right of it
      Serial.println("Is the corridor on the left or right?");
      char side = Serial.read();
      // Wait until user has picked left or right before setting on object
      // Then continuing to display message to the user of what they have selected
      while(side != 'L' || side != 'R') {
        side = Serial.read();
        
        if(side == 'L') {
          corridors.back().setSide('L');
          break;                        
        }
        if(side == 'R') {
          corridors.back().setSide('R');  
          break;      
        }    
      }      
      Serial.println("New corridor about to be entered on " + String(corridors.back().getSide()) + " - Corridor ID: " + String(corridors.back().getID()));

      // Turn on sub corridor flag so Zumo knows this new corridor is a sub corridor
      corridors.back().setSubCorridorFlag();
      
    }

    // Scan room button implemented so scan can take place after user has moved Zumo in to room
    if(valFromGUI == 'B') {
      // Zumo will scan entire room for people by sweeping both left and right with ultra sonic sensor running
      performRoomScan();
    }

    // Signal end of sub corridor has been reached so Zumo can turn around and exit
    if(valFromGUI == 'Q') {
      // Do 180 degree turn
      motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
      delay(TURN_180_DURATION);
      motors.setSpeeds(0, 0);

      turnComplete = true;

      delay(5000);
      
      goForwardWithBorderDetectUntilCornerReached();    
            
    }

    // Signal that the end of the track has been reached
    if(valFromGUI == 'E') {
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
      // If Zumo is in a sub corridor
      // We use 'currentCorridor-1' as vector stores corridor 1 in position 0, corridor 2 in pos 1 etc.
      else if(corridors[currentCorridor-1].getSubCorridorFlag()) {        
        // Checks if Zumo has already completed its 180 turn or not
        // If it hasn't, it is reaching the end of the sub corridor
        if(!turnComplete) {
          Serial.println("Zumo has reached the end of the sub corridor");          
        }
        else {
          // Get sub corridors side it was entered on
          // It will need to turn the same way out of the sub corridor to continue its search 
          char wayToTurn = corridors[currentCorridor-1].getSide();

          // Set current corridor back to previous corridor as Zumo will now have exited back on to this corridor
          currentCorridor = corridors[currentCorridor-1].getPreviousCorridorID();

          // Sends a message to GUI to disable one of the turn buttons
          if(wayToTurn == 'L') {
            Serial.println("Disable_RightTurn");
          }
          else if(wayToTurn == 'R') {
            Serial.println("Disable_LeftTurn");
          }

          Serial.println("Zumo has exited the sub corridor and can now only turn " + String(wayToTurn) + " on to corridor " + String(currentCorridor));

          // Return turn complete boolean flag back to false ready for next sub corridor to use
          turnComplete = false;
        }
        
      }
      else {
        // Send message that a corner has been reached to GUI     
        Serial.println("Zumo has reached a corner.");        
      }
}

void performRoomScan() {
  // For loop to make Zumo sweep right and scan for objects whilst there is not an object detected
  // Zumo will stop scanning as soon as it finds an object, if one exists
  for(int i = 0; i < 10; i++) {
    if(!rooms.back().getObjectFound()) {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(50);
      motors.setSpeeds(0, 0);

      // Detect object function contains the actual object detection code via the ultra-sonic sensor
      detectObject();           
    }          
  }

  // This will make the Zumo sweep back again and out to the left to keep scanning if it's not already found an object
  for(int i = 0; i < 20; i++) {
    if(!rooms.back().getObjectFound()) {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(50);
      motors.setSpeeds(0, 0);

      detectObject();        
    }          
  }

  // Displays appropriate message to GUI depending on whether it found an object or not
  if(rooms.back().getObjectFound()) {
    Serial.println("Room scan complete - Object FOUND in room " + String(rooms.back().getID()));        
  }
  else {
    Serial.println("Room scan complete - Object NOT FOUND");
  }
}

void detectObject() {
  // https://gist.github.com/flakas/3294829
  
  long duration, cm;
  
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);

  // Set to 17cm as room depth is 15cm (added extra 2cm to allow for object being on outer edge of room)
  if(cm < 17) {
    rooms.back().setObjectFound(true);
  }    
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
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
