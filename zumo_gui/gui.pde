/* =========================================================
 * ====                   WARNING                        ===
 * =========================================================
 * The code in this tab has been generated from the GUI form
 * designer and care should be taken when editing this file.
 * Only add/edit code inside the event handlers i.e. only
 * use lines between the matching comment tags. e.g.

 void myBtnEvents(GButton button) { //_CODE_:button1:12356:
     // It is safe to enter your event code here  
 } //_CODE_:button1:12356:
 
 * Do not rename this tab!
 * =========================================================
 */

public void wBtn_click(GButton source, GEvent event) { //_CODE_:wBtn:471796:
  println("wBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("W");
} //_CODE_:wBtn:471796:

public void aBtn_click(GButton source, GEvent event) { //_CODE_:aBtn:529102:
  println("aBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("A");
} //_CODE_:aBtn:529102:

public void sBtn_click(GButton source, GEvent event) { //_CODE_:sBtn:487489:
  println("sBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("S");
} //_CODE_:sBtn:487489:

public void dBtn_click(GButton source, GEvent event) { //_CODE_:dBtn:325743:
  println("dBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("D");
} //_CODE_:dBtn:325743:

public void responseTxt_change(GTextArea source, GEvent event) { //_CODE_:responseTextArea:801252:
  println("responseTextArea - GTextArea >> GEvent." + event + " @ " + millis());
} //_CODE_:responseTextArea:801252:

public void completeBtn_click(GButton source, GEvent event) { //_CODE_:completeBtn:917622:
  println("completeBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("C");
} //_CODE_:completeBtn:917622:

public void roomBtn_click(GButton source, GEvent event) { //_CODE_:roomBtn:562327:
  println("roomBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("Ro");
} //_CODE_:roomBtn:562327:

public void corridorBtn_click(GButton source, GEvent event) { //_CODE_:corridorBtn:589553:
  println("corridorBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("Co");
} //_CODE_:corridorBtn:589553:

public void endBtn_click(GButton source, GEvent event) { //_CODE_:endBtn:377588:
  println("endBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("E");
} //_CODE_:endBtn:377588:

public void scanRoomBtn_click(GButton source, GEvent event) { //_CODE_:scanRoomBtn:298768:
  println("scanRoomBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("Scan");
} //_CODE_:scanRoomBtn:298768:

public void reverseBtn_click(GButton source, GEvent event) { //_CODE_:reverseBtn:406475:
  println("reverseBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("Rev");
} //_CODE_:reverseBtn:406475:

public void leftBtn_click(GButton source, GEvent event) { //_CODE_:leftBtn:233461:
  println("leftBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write('L');
} //_CODE_:leftBtn:233461:

public void rightBtn_click(GButton source, GEvent event) { //_CODE_:rightBtn:882484:
  println("rightBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write('R');
} //_CODE_:rightBtn:882484:

public void endSubCorridorBtn_click(GButton source, GEvent event) { //_CODE_:endSubCorridorBtn:237230:
  println("endSubCorridorBtn - GButton >> GEvent." + event + " @ " + millis());
  port.write("EndSub");
} //_CODE_:endSubCorridorBtn:237230:



// Create all the GUI controls. 
// autogenerated do not edit
public void createGUI(){
  G4P.messagesEnabled(false);
  G4P.setGlobalColorScheme(GCScheme.BLUE_SCHEME);
  G4P.setCursor(ARROW);
  surface.setTitle("Sketch Window");
  wBtn = new GButton(this, 80, 15, 61, 51);
  wBtn.setText("W");
  wBtn.setTextBold();
  wBtn.addEventHandler(this, "wBtn_click");
  aBtn = new GButton(this, 12, 73, 62, 51);
  aBtn.setText("A");
  aBtn.setTextBold();
  aBtn.addEventHandler(this, "aBtn_click");
  sBtn = new GButton(this, 81, 72, 62, 53);
  sBtn.setText("S");
  sBtn.setTextBold();
  sBtn.addEventHandler(this, "sBtn_click");
  dBtn = new GButton(this, 150, 71, 64, 54);
  dBtn.setText("D");
  dBtn.setTextBold();
  dBtn.addEventHandler(this, "dBtn_click");
  responseTextArea = new GTextArea(this, 145, 236, 328, 80, G4P.SCROLLBARS_NONE);
  responseTextArea.setText("Hi from Zumo");
  responseTextArea.setOpaque(true);
  responseTextArea.addEventHandler(this, "responseTxt_change");
  textAreaLbl = new GLabel(this, 262, 198, 124, 47);
  textAreaLbl.setTextAlign(GAlign.CENTER, GAlign.MIDDLE);
  textAreaLbl.setText("Message from Zumo");
  textAreaLbl.setOpaque(false);
  completeBtn = new GButton(this, 334, 27, 75, 43);
  completeBtn.setText("Complete");
  completeBtn.setTextBold();
  completeBtn.addEventHandler(this, "completeBtn_click");
  roomBtn = new GButton(this, 277, 84, 80, 30);
  roomBtn.setText("Room");
  roomBtn.setTextBold();
  roomBtn.addEventHandler(this, "roomBtn_click");
  corridorBtn = new GButton(this, 374, 84, 80, 30);
  corridorBtn.setText("Corridor");
  corridorBtn.setTextBold();
  corridorBtn.addEventHandler(this, "corridorBtn_click");
  endBtn = new GButton(this, 16, 276, 102, 30);
  endBtn.setText("End of Track");
  endBtn.setTextBold();
  endBtn.addEventHandler(this, "endBtn_click");
  scanRoomBtn = new GButton(this, 320, 156, 92, 32);
  scanRoomBtn.setText("Scan Room");
  scanRoomBtn.setTextBold();
  scanRoomBtn.addEventHandler(this, "scanRoomBtn_click");
  reverseBtn = new GButton(this, 76, 133, 72, 26);
  reverseBtn.setText("Reverse");
  reverseBtn.setTextBold();
  reverseBtn.addEventHandler(this, "reverseBtn_click");
  leftBtn = new GButton(this, 276, 119, 80, 30);
  leftBtn.setText("Left");
  leftBtn.setTextBold();
  leftBtn.addEventHandler(this, "leftBtn_click");
  rightBtn = new GButton(this, 373, 119, 80, 30);
  rightBtn.setText("Right");
  rightBtn.setTextBold();
  rightBtn.addEventHandler(this, "rightBtn_click");
  endSubCorridorBtn = new GButton(this, 15, 237, 103, 31);
  endSubCorridorBtn.setText("End of Sub Corridor");
  endSubCorridorBtn.setTextBold();
  endSubCorridorBtn.addEventHandler(this, "endSubCorridorBtn_click");
}

// Variable declarations 
// autogenerated do not edit
GButton wBtn; 
GButton aBtn; 
GButton sBtn; 
GButton dBtn; 
GTextArea responseTextArea; 
GLabel textAreaLbl; 
GButton completeBtn; 
GButton roomBtn; 
GButton corridorBtn; 
GButton endBtn; 
GButton scanRoomBtn; 
GButton reverseBtn; 
GButton leftBtn; 
GButton rightBtn; 
GButton endSubCorridorBtn; 