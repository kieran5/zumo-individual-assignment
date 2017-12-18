// Import required for G4P library to function
import g4p_controls.*;

// Import to set up serial port to send and recieve messages
import processing.serial.*;

// Initialising serial port variable
Serial port;

// Initialise variable to store incoming data from Arduino
String incomingVal;

// Initialise firstConnection flag
boolean firstConnection = false;

public void setup() {
  size(480, 320);
  createGUI();
  
  // Intialise variable with PORT and baud rate
  port = new Serial(this, "COM12", 9600);
  
  // Buffers until '\n' is received (which will be the end of each Serial.println() received from Zumo)
  // This means every '\n' received triggers the serialEvent function to be called 
  port.bufferUntil('\n');
  
}

public void draw() {
  // Draws GUI built in G4P GUI Builder
  background(230);
}

void serialEvent(Serial port) {
  // Read in string from Serial buffer until '\n' reached. This will be the end of each message sent as we use println
  incomingVal = port.readStringUntil('\n');
  
  // Check a valid value has been received
  if(incomingVal != null) {
    
    // Remove whitespace from beginning and end of string
    incomingVal = trim(incomingVal);
    println(incomingVal);
    
    // Ignore any Sensor readings printed
    if(!incomingVal.startsWith("Sens")) {
      // Check incoming value to see if first connection
      if(!firstConnection) {
        if(incomingVal.equals("Z")) {
          
          // Clear port ready for incoming data
          port.clear();
          
          firstConnection = true;
          
          // Tell Zumo that the GUI is awaiting a value
          port.write("Z");
          println("Connection made");
        }
      }
      else {
        // Update text area with response from Zumo
        responseTextArea.setText(incomingVal);     
      
        // Tell Zumo that the GUI is awaiting a value
        port.write("Z");
      }      
    }         
  }
}