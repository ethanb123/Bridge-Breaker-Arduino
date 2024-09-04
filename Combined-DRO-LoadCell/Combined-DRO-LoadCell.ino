#include "HX711.h"
#define DOUT  4
#define CLK  5

HX711 scale;

float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup

#define pinA 2  // Interrupt pin 0 (INT0)
#define pinB 3 // Interrupt pin 1 (INT1)
#define pinR 8  // Digital pin for reference pulse

volatile long position = 0;  // Track the position
volatile bool direction = true; // True for forward, false for backward
volatile bool referenceHit = false; // To detect the reference pulse

// Store the previous state of A and B
volatile bool lastA = LOW;
volatile bool lastB = LOW;

// Conversion factor based on the given range
const float conversionFactor = 250.0 / -27500.0;

void setup() {
  Serial.begin(9600);

  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinMode(pinR, INPUT_PULLUP); // Assuming active low reference signal

  lastA = digitalRead(pinA);
  lastB = digitalRead(pinB);

  attachInterrupt(digitalPinToInterrupt(pinA), readQuadrature, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), readQuadrature, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinR), referencePulse, FALLING);



  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void loop() {
  // Convert the position to millimeters
  float positionMM = position * conversionFactor;

  Serial.print("Position: ");
  Serial.print(positionMM);
  Serial.print(" mm ");
  //Serial.print("\tReference: ");
  //println(referenceHit ? "Hit" : "Not Hit");

  delay(100);

  




    scale.set_scale(calibration_factor); //Adjust to this calibration factor

    Serial.print("Weight: ");
    Serial.print(scale.get_units(), 1);
    Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    //Serial.print(" calibration_factor: ");
    //Serial.print(calibration_factor);
    Serial.println();

    if(Serial.available())
    {
      char temp = Serial.read();
      if(temp == '+' || temp == 'a')
        calibration_factor += 10;
      else if(temp == '-' || temp == 'z')
        calibration_factor -= 10;
    }
}

void readQuadrature() {
  bool currentA = digitalRead(pinA);
  bool currentB = digitalRead(pinB);

  if (currentA != lastA || currentB != lastB) {
    // Determine direction based on the change in A and B states
    if (lastA == LOW && currentA == HIGH) {
      if (currentB == LOW) {
        position++;
      } else {
        position--;
      }
    } else if (lastA == HIGH && currentA == LOW) {
      if (currentB == HIGH) {
        position++;
      } else {
        position--;
      }
    }

    lastA = currentA;
    lastB = currentB;
  }
}

void referencePulse() {
  referenceHit = true;
}
