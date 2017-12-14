// Import required for G4P library to function
import g4p_controls.*;

// Import to set up serial port to send and recieve messages
import processing.serial.*;

// Initialising serial port variable
Serial port;

// Initialise variable to store incoming data from Arduino
String incomingVal;

public void setup() {
  size(480, 320);
  createGUI();
  
  // Intialise variable with PORT and baud rate
  port = new Serial(this, "COM12", 9600);
  
  port.bufferUntil('\n');
  
}

public void draw() {
  background(230);
}

void serialEvent(Serial port) {
  // Read in string until '\n' reached. This will be the end of each message sent as we use println
  incomingVal = port.readStringUntil('\n');
  
  if(incomingVal != null) {
    
    incomingVal = trim(incomingVal);
    println(incomingVal);
    responseTextArea.setText(incomingVal);
    
    port.clear();
      
  }
}