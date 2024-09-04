
// Constants for LED and switch pins
const int ledPin = 13;  // Pin for LED
const int switchPin = 12;  // Pin for switch

// Variable to hold switch state
int switchState = 0;

void setup() {
  // Set the LED pin as an output
  pinMode(ledPin, OUTPUT);
  
  // Set the switch pin as an input
  pinMode(switchPin, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input
}

void loop() {
  // Read the state of the switch
  switchState = digitalRead(switchPin);

  // If the switch is pressed (LOW state)
  if (switchState == LOW) {
    // Turn on the LED
    for(int i=0; i<5; i++) {
      digitalWrite(ledPin, LOW);
      delay(200);
      digitalWrite(ledPin, HIGH);
      delay(200);
    }
    
  } else {
    // Otherwise, turn off the LED
    digitalWrite(ledPin, HIGH);
  }
}
