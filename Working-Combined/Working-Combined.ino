const int ButtonUP = 12;
const int ButtonDOWN = 11;
const int ButtonGreen = 8;
const int ButtonRed = 9;

const int Relay1 = 6;
const int Relay2 = 10;

bool pushed_up = true;
bool pushed_down = true;
bool pushed_green = true;
bool pushed_red = true;



#include "HX711.h"
#define DOUT  4
#define CLK  5

HX711 scale;

float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup

#define pinA 2  // Interrupt pin 0 (INT0)
#define pinB 3 // Interrupt pin 1 (INT1)
#define pinR 8  // Digital pin for reference pulse

#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 4); // Default address of most PCF8574 modules, change according

volatile long position = 0;  // Track the position
volatile bool direction = true; // True for forward, false for backward
volatile bool referenceHit = false; // To detect the reference pulse

// Store the previous state of A and B
volatile bool lastA = LOW;
volatile bool lastB = LOW;

// Conversion factor based on the given range
const float conversionFactor = 250.0 / -27500.0;

float max_position = 0.0;
float max_weight = 0.0;


void setup (void)
{
  pinMode (ButtonUP, INPUT_PULLUP);
  pinMode (ButtonDOWN, INPUT_PULLUP);
  pinMode (ButtonGreen, INPUT_PULLUP);
  pinMode (ButtonRed, INPUT_PULLUP);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);


  lcd.begin(); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  lcd.backlight();

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
}

void loop (void)
{
  // Convert the position to millimeters
  float positionMM = position * conversionFactor;


  bool prev_up = pushed_up;
  bool prev_down = pushed_down;
  bool prev_green = pushed_green;
  bool prev_red = pushed_red;

  pushed_up = digitalRead (ButtonUP) == LOW;
  pushed_down = digitalRead (ButtonDOWN) == LOW;
  pushed_green = digitalRead (ButtonGreen) == LOW;
  pushed_red = digitalRead (ButtonRed) == LOW;


  lcd.setCursor(0, 0);
  //lcd.clear(0,0);
  lcd.print("Position: ");
  lcd.print(positionMM);
  lcd.print(" mm");

  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  lcd.setCursor(0, 1);
  lcd.print("Weight: ");
  lcd.print(scale.get_units(), 2);
  lcd.print(" lbs");
  //delay(800);
  


 if (prev_up != pushed_up)
    {
      if (pushed_up && !pushed_down) // Prevents multiple button pushed
        {
          digitalWrite(Relay1, LOW);
        }
      else                                                          
        {
          digitalWrite(Relay1, HIGH);
        }
    }

 if (prev_down != pushed_down)
    {
      if (pushed_down && !pushed_up) // Prevents multiple button pushed                               
        {
          digitalWrite(Relay2, LOW);
        }
      else                                                      
        {
          digitalWrite(Relay2, HIGH);
        }
    }

 if (prev_green != pushed_green)
    {
      if (pushed_green)   
        {
          position = 0;
          max_position = 0;
          max_weight = 0.0;
          for(int i=0; i<1; i++) {
            digitalWrite(Relay1, LOW);
            delay(200);
            digitalWrite(Relay1, HIGH);
            delay(200);
          }
        }
      else  
        {
          digitalWrite(Relay1, HIGH);
        }
    }

   if (prev_red != pushed_red)
    {
      if (pushed_red)    
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TEST RUN");
            
            lcd.setCursor(0, 1);
            
            lcd.print("Max Distance: ");
            lcd.print(max_position);
            lcd.print("mm");

            scale.set_scale(calibration_factor); //Adjust to this calibration factor

            lcd.setCursor(0, 2);
            lcd.print("Max Weight:   ");
            lcd.print(max_weight);
            lcd.print("lb");
            delay(5000);
            lcd.clear();
          /*for(int i=0; i<1; i++) {
            digitalWrite(Relay2, LOW);
            delay(200);
            digitalWrite(Relay2, HIGH);
            delay(200);
          }*/
        }
      else  
        {
          //digitalWrite(Relay2, HIGH);
        }
    }

    if (scale.get_units() >= max_weight)
    {
      max_weight = scale.get_units();
    }

    if (positionMM >= max_position)
    {
      max_position = positionMM;
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