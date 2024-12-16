#include "HX711.h"

#define calibration_factor -42000.0 // This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  2
#define CLK  3

HX711 scale;

unsigned long previousMillis = 0; // Will store the last time scale reading was taken
const long interval = 100; // Interval to read the scale every 1000 milliseconds (1 second)

void setup() {
  Serial.begin(9600);

  scale.begin(DOUT, CLK);
  scale.set_scale(-calibration_factor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); // Assuming there is no weight on the scale at start up, reset the scale to 0

  Serial.println("Value");
}

void loop() {
  Serial.println(scale.get_units(), 3);  // Weight reading with 1 decimal point (in kgs)
}
