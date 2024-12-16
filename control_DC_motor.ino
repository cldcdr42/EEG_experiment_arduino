#include <SparkFun_TB6612.h>


// === Pinout for TB6612 driver used to control DC motor === //
#define AIN1 8 
#define AIN2 7
#define PWMA 5
#define STBY 2

// === Initialize data for DC motor === //
const int offsetA = 1;
Motor motor1 = Motor(AIN1, AIN2, PWMA, offsetA, STBY);

// === Initialize data to recieve angle rotation values from Serial Port === //
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

int rotationAngle = 500;  // variable used for angle rotation value recieved and parced from Serial port
// rotationAngle == 500 // Initial value during start of the program
// rotationAngle == 501 // Error during string parsing from serial
// rotationAngle + currentAngle <= 120 && rotationAngle + currentAngle >= 0 // acceptable turning angle

int currentAngle = 60; //  variable stores current angular position of DC motor. Starts from 60, changes between 0 and 120
int newAngleDif;

boolean newData = false; 

void setup() {
    Serial.begin(9600);
    Serial.println("<MCU is ready. Orth v. 0.3>");
    //calibrateDCMotor();
    currentAngle = 60;
}

void loop() {
    getAngleFromSerial();
    parseAngleFromSerial();

    if (newData) {
      newAngleDif = rotationAngle - currentAngle;
      currentAngle = rotationAngle;

      rotateMotor(newAngleDif);
      sendAngleToSerial();
      newData = false;
    }
}

void getAngleFromSerial() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  
  if (Serial.available() > 0) {
    rc = Serial.read();
    
    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void parseAngleFromSerial() {
    rotationAngle = 500;
    rotationAngle = atoi(receivedChars);

    //int newAngle = currentAngle + rotationAngle;
    //if (!(newAngle <= 120 && newAngle >= 0)) {
    //  rotationAngle = 501;
    //  newData = false;
    //}
}

void rotateMotor(int rotationAngle) {
  // Use of the drive function which takes as parameters speed
  // and optional duration.  A negative speed will cause it to go
  // backwards.  Speed can be from -255 to 255.  Also use of the 
  // brake function which takes no arguements.
  int speed = 255;

  // to rotate backwards (negative angle) make speed value negative
  if (rotationAngle < 0) {
    speed *= -1;
  }

  // 36.7 - coefficient. Per 36,7 ms motor turns for 1 degree.
  motor1.drive(speed, 36.7*abs(rotationAngle));
  motor1.brake();

  //currentAngle += rotationAngle;
}

void sendAngleToSerial() {
  Serial.println(currentAngle);
}

void calibrateDCMotor() {
  rotateMotor(-140);
  rotateMotor(60);
}
