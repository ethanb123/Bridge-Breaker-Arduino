const int ButtonUP = 6;
const int ButtonDOWN = 7;
const int ButtonGreen = 49;
const int ButtonRed = 9;

const int Relay1 = 10;
const int Relay2 = 11;
const int Relay3 = 12;
const int Relay4 = 48;

bool pushed_up = true;
bool pushed_down = true;
bool pushed_green = true;
bool pushed_red = true;

#include <SD.h>
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
const float conversionFactor = 250.0 / 27500.0;

float max_position = 0.0;
float max_weight = 0.0;

const int chipSelect = 53;
File myFile;

//double logArray[2][200];
char filename[] = "LOG-001.csv";
int logStartTime = 0;
bool isLogging = false;

void setup (void)
{
  Serial.begin(9600);

  pinMode (ButtonUP, INPUT_PULLUP);
  pinMode (ButtonDOWN, INPUT_PULLUP);
  pinMode (ButtonGreen, INPUT_PULLUP);
  pinMode (ButtonRed, INPUT_PULLUP);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);

  digitalWrite(Relay1, LOW);
  digitalWrite(Relay2, LOW);
  digitalWrite(Relay3, LOW);
  digitalWrite(Relay4, LOW);


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

  scale.set_scale(calibration_factor); //Adjust to this calibration factor
}

void loop (void)
{
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Card Not Detected");
    delay(3000);
    lcd.clear();
  }else{
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

    // Print Distance on LCD
    lcd.setCursor(0, 0);
    lcd.print("Distance:");
    lcd.setCursor(10, 0);
    lcd.print(positionMM);
    lcd.print(" mm");

    

    // Print Weight on LCD
    lcd.setCursor(0, 1);
    lcd.print("Weight:");
    lcd.setCursor(10, 1);
    float currentWeight = scale.get_units();
    // Disable negative weights (Makes output hard to read)
    if(currentWeight < 0) {
      lcd.print(0.00);
    }else{
      lcd.print(currentWeight, 2);
    }
    lcd.print(" lbs");

    // Up button pushed
    if (prev_up != pushed_up)
        {
          if (pushed_up && !pushed_down) // Prevents multiple button pushed
            {
              digitalWrite(Relay1, HIGH);
            }
          else                                                          
            {
              digitalWrite(Relay1, LOW);
            }
        }

    // Down button pushed
    if (prev_down != pushed_down)
        {
          if (pushed_down && !pushed_up) // Prevents multiple button pushed                               
            {
              digitalWrite(Relay2, HIGH);
            }
          else                                                      
            {
              digitalWrite(Relay2, LOW);
            }
        }

    // Green button pushed
    if (prev_green != pushed_green)
      {
        if (pushed_green)   
          {
            digitalWrite(Relay4, LOW); // turn off Red light
            digitalWrite(Relay3, HIGH);// turn on Green light
            
            newFile();
            isLogging = true;
            max_position = 0.0;
            max_weight = 0.0;
            position = 0;
            positionMM = 0;

            digitalWrite(Relay2, HIGH);// Turn on Motor Down
          }
        else  
          {
            //digitalWrite(Relay1, LOW);
            //digitalWrite(Relay3, LOW);
          }
      }

    // Red button pushed
    if (prev_red != pushed_red)
    {
      if (pushed_red)    
        {
          if(isLogging){
            digitalWrite(Relay2, LOW); // turn motor off
            endLog();
            digitalWrite(Relay4, HIGH); // Turn on Red Light
            delay(5);
            digitalWrite(Relay4, LOW); // Turn off Red light
          }
          position = 0;
          digitalWrite(Relay4, HIGH); // Turn on Red Light
          delay(1);
          digitalWrite(Relay4, LOW); // Turn off Red light
        }
      else  
        {
          //digitalWrite(Relay4, LOW);
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

    int logTime = millis()-logStartTime;
    double logPosition = positionMM;
    double logWeight = scale.get_units();

    // CSV data entry
    myFile.println(String(logTime)+","+String(logPosition)+","+String(logWeight));

    // Display if currently logging data
    if(isLogging){
      
      
      lcd.setCursor(0,2);
      lcd.print("Logging... "+String(logTime/1000)+" Sec");

      if( (scale.get_units()+5) < max_weight) {
        digitalWrite(Relay2, LOW);
        isLogging = false;
        endLog();
        delay(5);
        digitalWrite(Relay4, LOW);
      }
    }
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

void endLog() {
  digitalWrite(Relay3, LOW);
          digitalWrite(Relay4, HIGH);
          lcd.clear();
          
          // if the file opened okay, write to it:
          if (myFile) {
            //Serial.print("Writing to test.txt...");
            lcd.setCursor(0, 0);
            lcd.print("Log Number: "+String(filename[4])+String(filename[5])+String(filename[6]));
            myFile.println("Max Distance,"+String(max_position));
            myFile.println("Max Weight,"+String(max_weight));
            // close the file:
            myFile.close();
            //Serial.println("done.");

            // Print max distance to LCD
            lcd.setCursor(0, 1);
            lcd.print("Distance:");
            lcd.setCursor(10, 1);
            lcd.print(max_position);
            lcd.print(" mm");

            scale.set_scale(calibration_factor); //Adjust to this calibration factor
            // ^^ Check if can remove

            // Print max weight to LCD
            lcd.setCursor(0, 2);
            lcd.print("Weight:");
            lcd.setCursor(10, 2);
            lcd.print(max_weight);
            lcd.print(" lbs");
            delay(5000);
            lcd.clear();
          } else {
            // if the file didn't open, print an error:
            lcd.print("error opening log");
            delay(3000);
            lcd.clear();
          }
          isLogging=false;
}

void newFile() {
  
  for (uint16_t i = 1; i < 1000; i++) {
      filename[4] = i / 100 + '0';       // Hundreds place
      filename[5] = (i / 10) % 10 + '0'; // Tens place
      filename[6] = i % 10 + '0';        // Units place
      if (!SD.exists(filename)) {
          // Only open a new file if it doesn't exist
          myFile = SD.open(filename, FILE_WRITE); 
          logStartTime = millis();
          myFile.println("Log Number," + String(filename[4]) + String(filename[5]) + String(filename[6]));
          myFile.println("Time,Position,Weight");
          break;  // Leave the loop
      }
  }

}


void referencePulse() {
  referenceHit = true;
}