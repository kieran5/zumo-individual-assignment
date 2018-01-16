#include <ZumoMotors.h>
#include <Servo.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>
#include <NewPing.h>

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
#define TURN_SPEED        80
#define FORWARD_SPEED     150
#define REVERSE_DURATION  100
#define TURN_DURATION     200
#define FORWARD_DURATION  100

#define TURN_180_SPEED         400
#define TURN_180_DURATION      400

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors(QTR_NO_EMITTER_PIN);
ZumoMotors motors;

// Initialise sensor variables to store values from reflectance sensors
#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

// Assign trig and echo pin numbers for ultra sonic sensor
const int trigPin = 2;
const int echoPin = 6;

// Initialise NewPing variable for use in room scan function
NewPing sonar(trigPin, echoPin, 200);

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

// Boolean value to allow Zumo to know when a new corridor instance is not needed
// This flag is required due to how the left and right turn buttons work for task 5
bool newCorridorNotRequired = false;


// Variable used for task 5 to hold duration recorded to be passed to appropriate objects
// A return journey flag so Zumo knows whether it is working its own way back to the beginning of the course
unsigned long durationVar = 0;
bool onReturnJourney = false;

class Corridor {
  private:
    int id;
    char corridorSide;
    bool subCorridorFlag;
    int previousCorridorID;
    unsigned long turnDuration;
    unsigned long totalDuration;
    unsigned long subCorridorDuration;

  public:
    Corridor();
    int getID();
    void setSide(char);
    char getSide();
    void setSubCorridorFlag();
    bool getSubCorridorFlag();
    void setPreviousCorridorID(int);
    int getPreviousCorridorID();
    void setTurnDuration(unsigned long);
    unsigned long getTurnDuration();
    void addToTotalDuration(unsigned long);
    void deductFromTotalDuration(unsigned long);
    unsigned long getTotalDuration();
};

Corridor::Corridor() {
  id = corridorCount;
  corridorSide = 'A';
  subCorridorFlag = false;
  totalDuration = 0;
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

void Corridor::setTurnDuration(unsigned long duration) {
  turnDuration = duration;
}

unsigned long Corridor::getTurnDuration() {
  return this->turnDuration;
}

void Corridor::addToTotalDuration(unsigned long duration) {
  totalDuration += duration;  
}

void Corridor::deductFromTotalDuration(unsigned long duration) {
  totalDuration -= duration;
}

unsigned long Corridor::getTotalDuration() {
  return this->totalDuration;
}


class Room {
  private:
    int id;
    char roomSide;
    int corridorID;
    bool objectFound;
    unsigned long duration;

  public:
    Room();
    int getID();
    void setSide(char);
    char getSide();
    void setCorridor(int);
    int getCorridor();
    void setObjectFound(bool);
    bool getObjectFound();
    void setDuration(unsigned long);
    unsigned long getDuration();
};

Room::Room() {
  id = roomCount;
  roomSide = 'A';
  objectFound = false;
  duration = 0;
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

void Room::setDuration(unsigned long d) {
  duration = d;
}

unsigned long Room::getDuration() {
  return this->duration;
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

      // If a Zumo reaches a corner, the user will press A to turn left on to the next so we set 'L' as the corridors side
      // If it is a sub corridor, it will have already had a side set by a button press
      // This is why we check for the default value of 'A' below
      // This check will also prevent the side being changed when users are navigating manually for other reasons as well (moving in/out of room etc.)
      if(corridors[currentCorridor-1].getSide() == 'A' && !newCorridorNotRequired) {
        corridors[currentCorridor-1].setSide('L');

        // Create new corridor instance and store in vector
        createNewCorridor(); 
        
      }

      // Set start time so Zumo is able to calculate duration of turn
      unsigned long startTime = millis();

      // Reading serial point waiting for user to press stop when the Zumo has turned enough
      while(Serial.read() != 'S') {
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);          
      }

      // Stop the Zumo, and calculate duration, ready to store in an object for task 5
      motors.setSpeeds(0, 0);
      unsigned long turnDuration = millis() - startTime;      
      corridors.back().setTurnDuration(turnDuration);
    }

    // Stop Zumo (for when you reach a room or side-corridor)
    if(valFromGUI == 'S') {
      Serial.println("S received by Zumo!");
      motors.setSpeeds(0, 0);
      
    }

    // Turn Zumo to right slowly until stopped by user
    if(valFromGUI == 'D') {
      Serial.println("D received by Zumo!");

      if(corridors[currentCorridor-1].getSide() == 'A' && !newCorridorNotRequired) {
        corridors[currentCorridor-1].setSide('R');

        // Create new corridor instance and store in vector
        createNewCorridor();
      }

      // Set start time so Zumo is able to calculate duration of turn
      unsigned long startTime = millis();

      // Reading serial point waiting for user to press stop when the Zumo has turned enough
      while(Serial.read() != 'S') {
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);          
      }

      // Stop the Zumo, and calculate duration, ready to store in an object for task 5
      motors.setSpeeds(0, 0);
      unsigned long turnDuration = millis() - startTime;      
      corridors.back().setTurnDuration(turnDuration);
    }

    // Reverse control for Zumo - in case needed when Zumo under human control
    if(valFromGUI == 'X') {
      Serial.println("Rev received by Zumo!");
      motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      delay(REVERSE_DURATION);
      motors.setSpeeds(0, 0);

    }

    // Signal that Zumo has completed turn and can carry on forward
    if(valFromGUI == 'C') {
      Serial.println("C received by Zumo!");

      // Return boolean flag back to false ready for next use
      // If it was set to true, then the user hitting complete will signify that
      // the corner or room search has been dealt with and it is safe to remove this flag
      newCorridorNotRequired = false;
      
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

      // Set duration (milliseconds) between last stop and the new room
      rooms.back().setDuration(durationVar);
            
      Serial.println("New room about to be entered on " + String(rooms.back().getSide()) + " in corridor " + String(rooms.back().getCorridor()) + " - Room ID: " + String(rooms.back().getID()));

      // We set this flag to true so that when we use the manual WASD controls to turn the Zumo in to a room, 
      // we won't activate the new corridor code required for task 5
      newCorridorNotRequired = true;
      
    }

    // Signal that a side-corridor is about to be entered
    if(valFromGUI == 'M') {
      Serial.println("Co received by Zumo!");

      // Create new corridor instance and store in vector
      createNewCorridor();
      
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

      // Set duration (milliseconds) between last stop and the new corridor
      corridors.back().addToTotalDuration(durationVar);
      
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

      // We need to activate this flag so Zumo knows it will be exiting back on to a corridor
      // that has already been created
      newCorridorNotRequired = true;

      delay(5000);
      
      goForwardWithBorderDetectUntilCornerReached();    
            
    }

    // Signal that the end of the track has been reached
    if(valFromGUI == 'E') {
      Serial.println("E received by Zumo!");

      onReturnJourney = true;

      // TEST OUTPUTS

      /*for(int i=0; i < corridors.size(); i++) {
        Serial.println("Corridor " + String(corridors[i].getID())); 
        Serial.println("Total duration: " + String(corridors[i].getTotalDuration()));
        Serial.println("Turn duration: " + String(corridors[i].getTurnDuration()));
        Serial.println("Have to turn " + String(corridors[i].getSide()) + " on to corridor " + String(corridors[i].getPreviousCorridorID()));
        if(corridors[i].getSubCorridorFlag()) Serial.println("Sub Corridor.");
        Serial.println();
        delay(1500);
      }
      
      for(int i=0; i < rooms.size(); i++) {
        Serial.println("Room " + String(rooms[i].getID()));
        Serial.println("Duration: " + String(rooms[i].getDuration()));
        Serial.println("On " + String(rooms[i].getSide()) + " of corridor " + String(rooms[i].getCorridor()));
        if(rooms[i].getObjectFound()) Serial.println("Object found.");
        Serial.println();
        delay(1500);
      }*/

      // CODE
      // Do 180 degree turn
      motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
      delay(TURN_180_DURATION);
      motors.setSpeeds(0, 0);

      delay(5000);
      
      while(corridors.size() > 1) {
        reflectanceSensors.read(sensor_values);
        while(!(sensor_values[0] > QTR_THRESHOLD && sensor_values[5] > QTR_THRESHOLD)) {
          
          // If next room to be dealt with is on current corridor then continue in to if statement, otherwise it will be a sub corridor coming up next
          if(rooms.back().getCorridor() == corridors[currentCorridor-1].getID()) {
            // If the room had an object in it, we need to check it on the way back to check if the person has now been saved
            if(rooms.back().getObjectFound()) {
              // Calculate duration to travel to last room on last corridor and stop
              motors.setSpeeds(0, 0);
              unsigned long duration = corridors[currentCorridor-1].getTotalDuration();
              duration -= rooms.back().getDuration();

              // Checks to see if corridor has both a room and a sub corridor and updates durations to accomodate this
              if(corridors.back().getPreviousCorridorID() == corridors[currentCorridor-1].getID()) {
                duration -= corridors.back().getTotalDuration();
                
                // Will deduct the duration to get to the room on the corridor from the corridors total duration
                // This is so my duration calculations work correctly when the Zumo has to calculate the distance between
                // the room and the sub corridor
                unsigned long roomDuration = rooms.back().getDuration();
                corridors[currentCorridor-1].deductFromTotalDuration(roomDuration);
              }

              // Deduct a little off of the duration calculation will always come out as a bit more than actually required
              duration - 250;
              motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
              delay(duration);
              motors.setSpeeds(0, 0);

              delay(2500);
          
              // Once robot reaches room, turn it in to room ready to scan
              if(rooms.back().getSide() == 'L') {
                // Turn robot right
                motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
                delay(TURN_180_DURATION / 2);
                motors.setSpeeds(0, 0);
                
              }
              else {
                // Turn robot left
                motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
                delay(TURN_180_DURATION / 2);
                motors.setSpeeds(0, 0);
                
              }
          
              // Reset last rooms object found attribute back to false and scan room again
              rooms.back().setObjectFound(false);
              performRoomScan();                    
              
              // If object still found then turn on led and send message to GUI
              if(rooms.back().getObjectFound()) {
                Serial.println("Person still awaiting rescue in room " + String(rooms.back().getID()) + ". FOLLOW ME!");
  
                // Beep for testing purposes
                buzzer.playNote(NOTE_G(4), 500, 15);
                
                // Flash LED twice before leaving on for person in need to follow
                digitalWrite(LED_PIN, HIGH);
                digitalWrite(LED_PIN, LOW);
                digitalWrite(LED_PIN, HIGH);
                digitalWrite(LED_PIN, LOW);
                digitalWrite(LED_PIN, HIGH);
              }
              else {
                Serial.println("Person in room " + String(rooms.back().getID()) + " has been SAVED!");
              }

              delay(1500);
          
              // Turn back out
              if(rooms.back().getSide() == 'L') {
                // Turn robot left
                motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
                delay(TURN_180_DURATION / 2);
                motors.setSpeeds(0, 0);              
              }
              else {
                // Turn robot right
                motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
                delay(TURN_180_DURATION / 2);
                motors.setSpeeds(0, 0);              
              }
            }

            delay(4000);
            
            // Remove room from vector as no longer needed
            rooms.pop_back();
          
          }
          // Means a sub corridor is coming up before the next room - need to check this, and check if the sub corridor contains a room with a person in
          else if(corridors.back().getPreviousCorridorID() == corridors[currentCorridor-1].getID() && rooms.back().getObjectFound()) {
            // Calculate duration to travel to last sub corridor on last corridor and stop
            motors.setSpeeds(0, 0);
            unsigned long duration = corridors[currentCorridor-1].getTotalDuration();
            duration -= corridors.back().getTotalDuration();
                          
            motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
            delay(duration);
            motors.setSpeeds(0, 0);

            delay(2000);
                
            // If the sub corridor contains a room of interest, turn towards sub corridor
            if(corridors.back().getSide() == 'L') {
              // Turn robot right
              motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0); 
            }
            else {
              // Turn robot left
              motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);         
            }

            delay(4000);
          
            // Find next room and search it again
            // Calculate duration to travel to last room on last corridor and stop
            duration = rooms.back().getDuration();
            motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
            delay(duration);
            motors.setSpeeds(0, 0);
        
            // Once robot reaches room, turn it in to room ready to scan
            if(rooms.back().getSide() == 'L') {
              // Turn robot left
              motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);
              
            }
            else {
              // Turn robot right
              motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);                
              
            }
        
            // Reset last rooms object found attribute back to false and scan room again
            rooms.back().setObjectFound(false);
            performRoomScan();                    
            
            // If object still found then turn on led and send message to GUI
            if(rooms.back().getObjectFound()) {
              Serial.println("Person still awaiting rescue in room " + String(rooms.back().getID()) + ". FOLLOW ME!");
              
              // Flash LED twice before leaving on for person in need to follow
              digitalWrite(LED_PIN, HIGH);
              digitalWrite(LED_PIN, LOW);
              digitalWrite(LED_PIN, HIGH);
              digitalWrite(LED_PIN, LOW);
              digitalWrite(LED_PIN, HIGH);
            }
            else {
              Serial.println("Person in room " + String(rooms.back().getID()) + " has been SAVED!");
            }
        
        
            // Turn back out
            if(rooms.back().getSide() == 'L') {
              // Turn robot left
              motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);              
            }
            else {
              // Turn robot right
              motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);              
            }

            delay(4000);
          
            // Remove room from vector as no longer needed
            rooms.pop_back();          
          
            // Drive back out of sub corrdidor
            motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
            delay(duration);
            motors.setSpeeds(0, 0);
          
            // Turn correct way to carry on return journey
            if(corridors.back().getSide() == 'L') {
              // Turn robot right
              motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);            
              
            }
            else {
              // Turn robot left
              motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
              delay(TURN_180_DURATION / 2);
              motors.setSpeeds(0, 0);              
            }

            delay(4000);
          }
          else {
            goForwardWithBorderDetectUntilCornerReached();       
          }

          reflectanceSensors.read(sensor_values);
        }
  
        // Line in front reached
        motors.setSpeeds(0, 0);

        delay(5000);
        
  
        // Update current corridor to previous one
        currentCorridor = corridors.back().getPreviousCorridorID();
  
        // Turn correct way depending on current corridor and previous corridor attributes
        if(corridors[currentCorridor-1].getSide() == 'L') {
          // Turn robot right
          motors.setSpeeds(TURN_180_SPEED, -TURN_180_SPEED);
          delay(TURN_180_DURATION / 2);
          motors.setSpeeds(0, 0);                                         
        }
        else { 
          // Turn robot left
          motors.setSpeeds(-TURN_180_SPEED, TURN_180_SPEED);
          delay(TURN_180_DURATION / 2);
          motors.setSpeeds(0, 0); 
        }
  
        // Remove last corridor from vector
        corridors.pop_back();


        delay(5000);
        
      }
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

    // Set start time so Zumo knows when it has started travelling up a corridor
    // Duration will be captured if it is required and saved to an object
    unsigned long startTime = millis();
    
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

        // Code to double check the Zumo hasn't hit a corner at an unusual angle
        if (sensor_values[1] > QTR_THRESHOLD || sensor_values[2] > QTR_THRESHOLD || sensor_values[3] > QTR_THRESHOLD || sensor_values[4] > QTR_THRESHOLD) {
          break;
        }

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

      // Calculate duration while loop was running so it can be assigned to the appropriate attribuite on the appropriate object
      unsigned long duration = millis() - startTime;

      // Add duration to total duration of current corridor
      // This is so Zumo knows how long to go forward on the return journey
      // We obviously only need to add to this attribute whilst on the initial journey
      // Need to make sure duration isn't added to a sub corridor as we don't care about the total duration of that
      // Only the duration the sub corridor is down it's parent corridor
      if(!onReturnJourney && !newCorridorNotRequired && !corridors[currentCorridor-1].getSubCorridorFlag()) {
        corridors[currentCorridor-1].addToTotalDuration(duration);        
      }
      

      // Once line in front of Zumo detected - will fall out of while loop and stop motors
      motors.setSpeeds(0, 0);
      
      // If else statement to check what message to display to user on GUI
      if(stopPressed) {
        Serial.println("Zumo stopped by user.");

        // Store duration in a duration variable for use when the room or corridor buttons are hit
        // On a room or corridor button press we will need this duration to store on a sub corridor or room object
        // This will be used in conjunction with the corridors totalDuration attribute
        // We will deduct the duration stored on a room or corridor object from the current corridors totalDuration in order
        // to figure out the duration the Zumo needs to go on its return journey...
        durationVar = duration;
        //Serial.println("duration: " + String(duration));
        //Serial.println("durationVar: " + String(durationVar));
        
      }
      // If Zumo is in a sub corridor
      // We use 'currentCorridor-1' as vector stores corridor 1 in position 0, corridor 2 in pos 1 etc.
      else if(corridors[currentCorridor-1].getSubCorridorFlag()) {        
        // Make use of newCorridorNotRequired flag - this flag will only be made true AFTER the turn at end of sub corridor
        // If Zumo hasn't completed its turn, it is reaching the end of the sub corridor
        if(!newCorridorNotRequired) {
          Serial.println("Zumo has reached the end of the sub corridor");

          // Zumo pulls back quickly to prepare to be turned by user
          motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
          delay(REVERSE_DURATION);
          motors.setSpeeds(0, 0);         
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

          // Zumo pulls back quickly to prepare to be turned by user
          motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
          delay(REVERSE_DURATION);
          motors.setSpeeds(0, 0); 
        }
        
      }
      else if(onReturnJourney) {
        Serial.println("Return journey complete, Zumo finished!");
      }
      else {
        // Send message that a corner has been reached to GUI     
        Serial.println("Zumo has reached a corner.");

        // Zumo pulls back quickly to prepare to be turned by user
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        delay(REVERSE_DURATION);
        motors.setSpeeds(0, 0);
      }
}

void performRoomScan() {
  // For loop to make Zumo sweep right and scan for objects whilst there is not an object detected
  // Zumo will stop scanning as soon as it finds an object, if one exists
  for(int i = 0; i < 5; i++) {
    if(!rooms.back().getObjectFound()) {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(500);
      motors.setSpeeds(0, 0);

      detectObject();
    }          
  }

  // This will make the Zumo sweep back again and out to the left to keep scanning if it's not already found an object
  for(int i = 0; i < 10; i++) {
    if(!rooms.back().getObjectFound()) {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(500);
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
  // Every time this function is called, the sensor will ping 10 times before it is moved on to the next position
  // It will break as soon as a object is found
  // Set to 20cm to allow for Zumo not being far in to the room and object being on far corner of room (room is 15cm deep)
  for(int i=0; i < 10; i++) {
    delay(50);
    if(sonar.ping_cm() < 20 && sonar.ping_cm() != 0) {
      rooms.back().setObjectFound(true);
      break;
    }
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

void createNewCorridor() {
  // Take note of previous corridor ID so new corridor can record this information for use when new corridor is exited again
  int prevCorridorID = currentCorridor;

  // Increment corridor count variable to assign new corridor with own unique ID
  ++corridorCount;
  corridors.push_back(Corridor::Corridor());

  // Update current corridor variable so Zumo knows where it is
  currentCorridor = corridors.back().getID();

  // Set previous corridor ID so Zumo can keep this in memory for when it exits back on to previous corridor
  corridors.back().setPreviousCorridorID(prevCorridorID);  
}
