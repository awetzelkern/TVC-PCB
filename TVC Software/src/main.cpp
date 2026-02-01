#include <Arduino.h>
#include <pins.h>
#include <SPI.h>

// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600); // baud rate = 9600 bits/s
  pinMode(MEM_CS, OUTPUT); // MCU can set this pin to high or low
  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  Serial.println("Hello World");

  digitalWrite(MEM_CS, LOW); // begins transaction
  SPI.beginTransaction({10000000, MSBFIRST, SPI_MODE0}); // frequency (10MHz), most significant bit first, default
  SPI.transfer(8);
  digitalWrite(MEM_CS, HIGH); //ends transmission
}
