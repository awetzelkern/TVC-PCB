#include <Arduino.h>
#include <pins.h>
#include <SPI.h>
#include <Servo.h>

// put function declarations here:
int xPos = 0; // variable to store the x-axis servo position
int yPos = 0; // variable to store the y-axis servo position

Servo xServo;
Servo yServo;

void setup() {
  // put your setup code here, to run once:

  // Servo Setup
  xServo.attach(TVCX);
  yServo.attach(TVCY);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600); // baud rate = 9600 bits/s
  pinMode(MEM_CS, OUTPUT); // MCU can set this pin to high or low
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  // Test Servo Movement
  for (yPos = 0; yPos <= 180; yPos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      yServo.write(yPos);              // tell servo to go to position in variable 'yPos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  for (yPos = 180; yPos >= 0; yPos -= 1) { // goes from 180 degrees to 0 degrees
    yServo.write(yPos);              // tell servo to go to position in variable 'yPos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  for (xPos = 0; xPos <= 180; xPos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    xServo.write(xPos);              // tell servo to go to position in variable 'xPos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (xPos = 180; xPos >= 0; xPos -= 1) { // goes from 180 degrees to 0 degrees
    xServo.write(xPos);              // tell servo to go to position in variable 'xPos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  digitalWrite(MEM_CS, LOW); // begins transaction
  SPI.beginTransaction({10000000, MSBFIRST, SPI_MODE0}); // frequency (10MHz), most significant bit first, default
  SPI.transfer(8);
  digitalWrite(MEM_CS, HIGH); //ends transmission
}
