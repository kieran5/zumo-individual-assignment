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

// Initialise sensor variables to store values from reflectance sensors
#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

// Assign trig and echo pin numbers for ultra sonic sensor
const int trigPin = 2;
const int echoPin = 6;


boolean firstTimeWPressed = false;

// Initialise corridor and room ID variables
// corridorID starts at 1 as the Zumo will start in a corridor (ID = 1)
// Zumo will not be in a room until it comes across one though, hence starting on 0
// These variables are needed so that the class instances are able to increment their ID's
int initCorridorID = 1;
int initRoomID = 0;

// Initialise current corridor variable - allows Zumo to always knows what corridor it is in
int currentCorridor = 0;

// Initialise variable to check when an object is detected in a room
bool objectFound = false;

class Corridor {
  private:
    int id;
    char corridorSide;

  public:
    Corridor();
    int getID();
    void setSide(char);
    char getSide();
};

Corridor::Corridor() {
  id = initCorridorID;
  corridorSide = 'A';
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
    void setObjectFound();
    bool getObjectFound();
};

Room::Room() {
  id = initRoomID;
  roomSide = 'A';
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

void Room::setObjectFound() {
  objectFound = true;
}

bool Room::getObjectFound() {
  return this->objectFound;
}

// Intialise object arrays to store each instance of a room or corridor that is create
// We can quickly access these arrays to get data for any particular room or corridor
// TODO: Move away from using arrays and make use of a collection of some form (vectors, lists?)
Room rooms[10];
Corridor corridors[10];


void setup() {
    // Initialise the reflectance sensors module
    reflectanceSensors.init();

    // Init LED pin to enable it to be turned on later
    pinMode(LED_PIN, OUTPUT);

    // Set up serial port to enable Arduino to communicate with GUI
    Serial.begin(9600);

    // Call method to create initial connection to GUI
    establishConnectionToGUI();

    // Zumo will start in a corridor, will create this first corridor instance and store in array
    corridors[initCorridorID] = Corridor::Corridor();
    currentCorridor = corridors[initCorridorID].getID();
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
      // If not first time 'W' pressed, this control will just be used for manual forward control
      else {
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        delay(FORWARD_DURATION);
        motors.setSpeeds(0, 0);           
      }
    }

    // Turn Zumo to left (good idea to change this to 90 degree turn?)
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

    // Turn Zumo to right (good idea to change this to 90 degree turn?)
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

      // Increment room ID variable to assign new room with own unique ID
      ++initRoomID;
      rooms[initRoomID] = Room::Room();
      
      // Tell zumo whether room is on left or right of it
      Serial.println("Is the room on the left or right?");
      char side = Serial.read();
      // Wait until user has picked left or right before setting on object
      // Then continuing to display message to the user of what they have selected
      while(side != 'L' || side != 'R') { 
        side = Serial.read();
        
        if(side == 'L') {
          rooms[initRoomID].setSide('L');
          break;                        
        }
        if(side == 'R') {
          rooms[initRoomID].setSide('R');  
          break;      
        }    
      }

      // Set the current corridor of the room entered so Zumo remembers where it is on the course
      rooms[initRoomID].setCorridor(currentCorridor);
            
      Serial.println("New room about to be entered on " + String(rooms[initRoomID].getSide()) + " in corridor " + String(rooms[initRoomID].getCorridor()) + " - Room ID: " + String(rooms[initRoomID].getID()));
    }

    // Signal that a side-corridor is about to be entered
    if(valFromGUI == "Co") {
      Serial.println("Co received by Zumo!");

      // Increment corridor ID variable to assign new corridor with own unique ID
      ++initCorridorID;
      corridors[initCorridorID] = Corridor::Corridor();

      // Update current corridor variable so Zumo knows where it is
      currentCorridor = corridors[initCorridorID].getID();
      
      // Tell zumo whether corridor is on left or right of it
      Serial.println("Is the corridor on the left or right?");
      char side = Serial.read();
      // Wait until user has picked left or right before setting on object
      // Then continuing to display message to the user of what they have selected
      while(side != 'L' || side != 'R') {
        side = Serial.read();
        
        if(side == 'L') {
          corridors[initCorridorID].setSide('L');
          break;                        
        }
        if(side == 'R') {
          corridors[initCorridorID].setSide('R');  
          break;      
        }    
      }      
      Serial.println("New corridor about to be entered on " + String(corridors[initCorridorID].getSide()) + " - Corridor ID: " + String(corridors[initCorridorID].getID()));
      
    }

    // Scan room button implemented so scan can take place after user has moved Zumo in to room
    if(valFromGUI == "Scan") {
      performRoomScan();
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

void performRoomScan() {
  // For loop to make Zumo sweep right and scan for objects whilst there is not an object detected
  // Zumo will stop scanning as soon as it finds an object, if one exists
  for(int i = 0; i < 10; i++) {
    if(!objectFound) {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(50);
      motors.setSpeeds(0, 0);

      // Detect object function contains the actual object detection code via the ultra-sonic sensor
      detectObject();           
    }          
  }

  // This will make the Zumo sweep back again and out to the left to keep scanning if it's not already found an object
  for(int i = 0; i < 20; i++) {
    if(!objectFound) {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(50);
      motors.setSpeeds(0, 0);

      detectObject();        
    }          
  }

  // Displays appropriate message to GUI depending on whether it found an object or not
  if(objectFound) {
    Serial.println("Room scan complete - Object FOUND in room " + String(rooms[initRoomID].getID()));
    rooms[initRoomID].setObjectFound();        
  }
  else {
    Serial.println("Room scan complete - Object NOT FOUND");
  }
}

void detectObject() {
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
  if(cm < 17)
  {
    objectFound = true;
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
