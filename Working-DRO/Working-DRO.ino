
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
