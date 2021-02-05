#include <SevSeg.h>

// Include Arduino Wire library for I2C
#include <Wire.h>
 
// Define Slave I2C Address
#define SLAVE_ADDR 9

int boost = 0;

SevSeg sevseg; //Initiate a seven segment controller object


void setup() {
    // SevSeg
    byte numDigits = 4;  
    byte digitPins[] = {2, 3, 4, 5};
    byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
    bool resistorsOnSegments = 0; 
    // variable above indicates that 4 resistors were placed on the digit pins.
    // set variable to 1 if you want to use 8 resistors on the segment pins.
    sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
    sevseg.setBrightness(90);
    sevseg.setChars("----");

    // I2C
    // Initialize I2C communications as Slave
    Wire.begin(SLAVE_ADDR);

    // Function to run when data received from master
    Wire.onReceive(updateBoost);
}
void loop() {
    sevseg.refreshDisplay(); // Must run repeatedly
}

void updateBoost() {
  while (0 < Wire.available()) {
    byte low = Wire.read();
    byte high = Wire.read();

    unsigned int boost = high * 256 + low;
    
    sevseg.setNumber(boost, 2);
  }
}
