/* Example code for HC-SR501 PIR motion sensor with Arduino. More info: www.makerguides.com */

#include <SPI.h>
#include <SD.h>

const int chipSelect = 14;
uint16_t manufac[6] = {0xCC, 0xCC, 0xCC, 0x0A, 0x39, 0x8F};

// Define connection pins:
#define pirPin 26
#define ledPin 13
// Create variables:
int val = 0;
bool motionState = false; // We start with no motion detected.
void setup() {
  // Configure the pins as input or output:
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);
   Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

}
void loop() {
  // Read out the pirPin and store as val:
  val = digitalRead(pirPin);
  Serial.println(val);
  // If motion is detected (pirPin = HIGH), do the following:
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH); // Turn on the on-board LED.
    // Change the motion state to true (motion detected):
    if (motionState == false) {
    //  Serial.println("Motion detected!");
      motionState = true;
    }
  }
  // If no motion is detected (pirPin = LOW), do the following:
  else {
    digitalWrite(ledPin, LOW); // Turn off the on-board LED.
    // Change the motion state to false (no motion):
    if (motionState == true) {
    //  Serial.println("Motion ended!");
      motionState = false;
    }
  }
   File dataFile = SD.open("/text.txt", FILE_APPEND);
   if (dataFile) {
      dataFile.println(val);
      dataFile.close();
      // print to the serial port too:
      Serial.println(val);
    }
    else {
      Serial.println("error opening text.txt");
    }
  delay(1000);
}
