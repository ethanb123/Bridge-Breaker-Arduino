
// Constants for LED and switch pins
const int Relay1 = 13;  // Pin for LED
const int Button1 = 12;  // Pin for switch

const int Button2 = 11;  // Pin for LED
const int Relay2 = 10;  // Pin for LED

const int Button3 = 9;  // Pin for LED
const int Button4 = 8;  // Pin for LED

// Variable to hold switch state
int switchState = 0;
int switchState2 = 0;
int switchState3 = 0;
int switchState4 = 0;

void setup() {
  // Set the LED pin as an output
  pinMode(Relay1, OUTPUT);
  
  // Set the switch pin as an input
  pinMode(Button1, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input

  // Set the switch pin as an input
  pinMode(Button2, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input

  // Set the switch pin as an input
  pinMode(Button3, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input

  // Set the switch pin as an input
  pinMode(Button4, INPUT_PULLUP);  // Using INPUT_PULLUP to prevent floating input

  // Set the LED pin as an output
  pinMode(Relay2, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // Read the state of the switch
  switchState = digitalRead(Button1);
  switchState2 = digitalRead(Button2);
  switchState3 = digitalRead(Button3);
  switchState4 = digitalRead(Button4);

  // If the switch is pressed (LOW state)
  if (switchState == LOW) {
    // Turn on the LED
    /*
     Serial.print("Zero factor: ");
    for(int i=0; i<5; i++) {
      digitalWrite(Relay1, LOW);
      delay(200);
      digitalWrite(Relay1, HIGH);
      delay(200);
    }*/
    digitalWrite(Relay1, LOW);
    
  } else {
    // Otherwise, turn off the LED
    digitalWrite(Relay1, HIGH);
  }


    // If the BUTTON2 is pressed (LOW state)
  if (switchState2 == LOW) {

    /*
    // Turn on the LED
     Serial.print("Zero factor: ");
    for(int i=0; i<5; i++) {
      digitalWrite(Relay2, LOW);
      delay(200);
      digitalWrite(Relay2, HIGH);
      delay(200);
    }*/
    digitalWrite(Relay2, LOW);
    
  } else {
    // Otherwise, turn off the LED
    digitalWrite(Relay2, HIGH);
  }

  if (switchState3 == LOW) {
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

  if (switchState4 == LOW) {
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
