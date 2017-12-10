#include <ZumoMotors.h>
#include <Servo.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>

#define LED_PIN 13
#define TURN_SPEED 200
#define SPEED 400

ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;

void setup()
{
    // Initialise the reflectance sensors module
    reflectanceSensors.init();

    // Init LED pin to enable it to be turned on later
    pinMode(LED_PIN, OUTPUT);

    // Set up serial monitor for inital testing purposes
    Serial.begin(9600);
    Serial.println("waiting...");
}

void loop()
{
  if (Serial.available()>0) {
        char motor = (char) Serial.read();
        int pin;
        Servo movingServo;
        String name ="";
        switch (motor){
           case 'w': case 'W': digitalWrite(LED_PIN, HIGH); motors.setLeftSpeed(1000); motors.setRightSpeed(1000); delay(2); break;
           case 'a': case 'A': digitalWrite(LED_PIN, HIGH); motors.setLeftSpeed(0); motors.setRightSpeed(1000); delay(2); break;
           case 's': case 'S': digitalWrite(LED_PIN, HIGH); motors.setLeftSpeed(-1000); motors.setRightSpeed(-1000); delay(2); break;
           case 'd': case 'D': digitalWrite(LED_PIN, HIGH); motors.setLeftSpeed(1000); motors.setRightSpeed(0); delay(2); break;
           case 'q': case 'Q': digitalWrite(LED_PIN, HIGH); motors.setLeftSpeed(0); motors.setRightSpeed(0); delay(2); break;
        }
  }
}
