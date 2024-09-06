const int ButtonUP = 12;
const int ButtonDOWN = 11;

const int ButtonGreen = 9;
const int ButtonRed = 8;

const int Relay1 = 13;
const int Relay2 = 10;

// Variable to hold switch state
int switchStateUP = 0;
int switchStateDOWN = 0;
int switchStateGreen = 0;
int switchStateRed = 0;

void setup() {
  // Set the switch pin as an input
  pinMode(ButtonUP, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input
  pinMode(ButtonDOWN, INPUT_PULLUP);
  pinMode(ButtonGreen, INPUT_PULLUP); 
  pinMode(ButtonRed, INPUT_PULLUP); 

  // Set the LED pin as an output
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // Read the state of the switch
  switchStateUP = digitalRead(ButtonUP);
  switchStateDOWN = digitalRead(ButtonDOWN);
  switchStateGreen = digitalRead(ButtonGreen);
  switchStateRed = digitalRead(ButtonRed);

  // If the up switch is pressed (LOW state)
  if (switchStateUP == LOW) {
    digitalWrite(Relay1, LOW);
    delay(1);
  } else {
    digitalWrite(Relay1, HIGH);
  }

  // If the ButtonDOWN is pressed (LOW state)
  if (switchStateDOWN == LOW) {
    digitalWrite(Relay2, LOW);
    delay(1);
  } else {
    digitalWrite(Relay2, HIGH);
  }

  if (switchStateGreen == LOW) {
    // Turn on the LED
    for(int i=0; i<5; i++) {
      digitalWrite(Relay2, LOW);
      delay(200);
      digitalWrite(Relay2, HIGH);
      delay(200);
    }
  } else {
    // Otherwise, turn off the LED
    digitalWrite(Relay2, HIGH);
  }

  if (switchStateRed == LOW) {
  // Turn on the LED
  for(int i=0; i<5; i++) {
    digitalWrite(Relay1, LOW);
    delay(200);
    digitalWrite(Relay1, HIGH);
    delay(200);
  }
  } else {
    // Otherwise, turn off the LED
    digitalWrite(Relay1, HIGH);
  }

}
